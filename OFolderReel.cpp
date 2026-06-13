/*

MIT License

Copyright (c) 2025 AnkA Interactive

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


#include "pch.h"
#include "OFolderReel.hpp"

#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"
#include "framework/Core/Paging/Pager.hpp"
#include "framework/Core/IO/Folder/OFolderImageScanner.hpp"
#include "framework/Graphics/Image/OWicImageThumbnailer.hpp"
#include "framework/Graphics/Image/DecodedImage.hpp"
#include "framework/GUI/Presentation/BitmapConversion.hpp"

#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <iterator>
#include <span>
#include <unordered_map>
#include <utility>

using namespace talay::GUI::Reel;

using anka::Core::Object::ObjectIdentity;
using anka::Core::Localization::OLocalizationService;
using anka::Core::Paging::Pager;
using anka::Core::IO::OFolderImageScanner;
using anka::Graphics::Image::OWicImageThumbnailer;

using winrt::hstring;
using winrt::box_value;
using winrt::unbox_value;
using winrt::Windows::Foundation::IInspectable;

using winrt::Microsoft::UI::Dispatching::DispatcherQueuePriority;
using winrt::Microsoft::UI::Xaml::BringIntoViewOptions;
using winrt::Microsoft::UI::Xaml::CornerRadius;
using winrt::Microsoft::UI::Xaml::HorizontalAlignment;
using winrt::Microsoft::UI::Xaml::RoutedEventArgs;
using winrt::Microsoft::UI::Xaml::Style;
using winrt::Microsoft::UI::Xaml::TextTrimming;
using winrt::Microsoft::UI::Xaml::UIElement;
using winrt::Microsoft::UI::Xaml::Controls::Border;
using winrt::Microsoft::UI::Xaml::Controls::Image;
using winrt::Microsoft::UI::Xaml::Controls::StackPanel;
using winrt::Microsoft::UI::Xaml::Controls::TextBlock;
using winrt::Microsoft::UI::Xaml::Controls::Primitives::ToggleButton;
using winrt::Microsoft::UI::Xaml::Media::Brush;
using winrt::Microsoft::UI::Xaml::Media::FontFamily;
using winrt::Microsoft::UI::Xaml::Media::Stretch;
using winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap;

namespace
{
   constexpr std::uint32_t k_thumbEdge   {200};     // thumbnail longest-side px
   constexpr double        k_thumbHeight {120.0};

   // Tiles decoded per page — roughly a screenful and a half of strip; keeps
   // opening a huge folder from stalling the UI thread on thousands of decodes.
   constexpr std::size_t k_pageSize {10};

   // Decoded-thumbnail memo cap (several pages' worth) — flipping between
   // visited pages stays free without letting a giant folder hoard memory.
   constexpr std::size_t k_thumbCacheCap {150};

   // Lexical, case-blind path equality: launch paths arrive in the shell's
   // spelling (slashes, casing) while the scan carries the disk's, and a
   // filesystem hit per candidate would be too slow for a large folder.
   bool samePath(const std::wstring& left, const std::wstring& right)
   {
      const std::wstring a {std::filesystem::path {left}.lexically_normal().wstring()};
      const std::wstring b {std::filesystem::path {right}.lexically_normal().wstring()};
      return CompareStringOrdinal(a.c_str(), -1, b.c_str(), -1, TRUE) == CSTR_EQUAL;
   }
}

// -----------------------------------------------------------------------------
// OFolderReel::Impl Definition
// -----------------------------------------------------------------------------
struct OFolderReel::Impl
{
   ObjectIdentity m_identity {L"FolderReelObject"};

   std::vector<std::wstring> m_formats;

   ReelControls m_ui;

   // Folder whose images currently back the strip, its scanned paths, and the
   // pager that windows them; only the current page ever becomes tiles.
   std::wstring              m_folder;
   std::vector<std::wstring> m_files;
   Pager                     m_pager;

   // Thumbnails decoded once per path and kept while the folder is open, so
   // revisiting a page never re-decodes; capped, and cleared on folder change.
   std::unordered_map<std::wstring, WriteableBitmap> m_thumbCache;

   // Path shown in the viewport, kept so page turns can re-check its tile.
   std::wstring m_currentPath;

   ImageActivatedCallback  m_onActivated;
   ExpandRequestedCallback m_onExpand;

   explicit Impl(std::vector<std::wstring> formats) :
      m_formats {std::move(formats)}
   {
      m_pager.setPageSize(k_pageSize);
   }

   // Bind the controls, wire the pager buttons, and put the bar into its real
   // (empty, both arrows disabled) state — the XAML defaults show both enabled.
   void attach(const ReelControls& controls)
   {
      m_ui = controls;
      m_ui.pagePrev.Click([this](auto&&, auto&&) { onPreviousPage(); });
      m_ui.pageNext.Click([this](auto&&, auto&&) { onNextPage(); });
      updatePagerBar();
   }

   // Reflect 'imagePath': rescan if its folder changed, page to its tile, and
   // rebuild the strip only when the visible window actually moved.
   void show(const std::wstring& imagePath)
   {
      m_currentPath = imagePath;

      const bool rescanned {showFolderOf(imagePath)};
      const bool paged     {m_pager.goToItem(indexOf(imagePath))};
      if (rescanned || paged)
         refreshPage();
      else
         updateSelection(imagePath);
   }

   // Rescan when the opened image lives in a new folder; true means the strip's
   // backing list was rebuilt (and the host was asked to reveal the panel).
   bool showFolderOf(const std::wstring& imagePath)
   {
      const std::wstring folder {std::filesystem::path {imagePath}.parent_path().wstring()};
      if (folder == m_folder)
         return false;

      m_folder = folder;
      loadFolder(imagePath);
      requestExpand();
      return true;
   }

   void loadFolder(const std::wstring& imagePath)
   {
      m_files = OFolderImageScanner::getInstance().scan(imagePath, m_formats);
      m_thumbCache.clear();
      m_pager.setItemCount(m_files.size());
      m_pager.first();
   }

   // Position of 'path' in the scanned list, compared spelling-blind; a path
   // that vanished since the scan just lands on the first page.
   std::size_t indexOf(const std::wstring& path) const
   {
      const auto match {[&path](const std::wstring& file) { return samePath(file, path); }};

      const auto found = std::ranges::find_if(m_files, match);
      if (found == m_files.end())
         return 0;

      return static_cast<std::size_t>(std::distance(m_files.begin(), found));
   }

   // Replace the strip with the current page's tiles only, in the scanner's
   // name order — decoding stays bounded by the page size.
   void renderPage(void)
   {
      auto thumbs = m_ui.strip.Children();
      thumbs.Clear();

      for (const auto& file : pageFiles())
         thumbs.Append(makeThumb(file));

      updatePagerBar();
   }

   std::span<const std::wstring> pageFiles(void) const
   {
      return std::span {m_files}.subspan(m_pager.firstItem(), m_pager.itemsOnPage());
   }

   //
   // Paging — the pager owns the arithmetic, this owns the controls.
   //

   void updatePagerBar(void)
   {
      m_ui.pageLabel.Text(hstring {pageLabelText()});
      m_ui.pagePrev.IsEnabled(m_pager.hasPrevious());
      m_ui.pageNext.IsEnabled(m_pager.hasNext());
   }

   // The localized "{} / {}" pattern arrives at run time, so this goes through
   // vformat rather than a compile-time checked format string.
   std::wstring pageLabelText(void) const
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};
      const std::size_t number {m_pager.pageNumber()};
      const std::size_t count  {m_pager.pageCount()};
      return std::vformat(loc.text(L"ReelPageLabel"), std::make_wformat_args(number, count));
   }

   // Turning a page re-renders the cached paths (no rescan) and re-checks the
   // open image's tile if its page just scrolled back into view.
   void onPreviousPage(void)
   {
      if (m_pager.previous())
         refreshPage();
   }

   void onNextPage(void)
   {
      if (m_pager.next())
         refreshPage();
   }

   // The old page's scroll offset survives the rebuild and would land the user
   // anywhere mid-strip: correct it to the open image's tile, or to the top of
   // a page that does not hold it.
   void refreshPage(void)
   {
      renderPage();

      if (!updateSelection(m_currentPath))
         m_ui.scroller.ChangeView(nullptr, 0.0, nullptr, true);
   }

   void requestExpand(void) const
   {
      if (m_onExpand)
         m_onExpand();
   }

   // Check the tile for the open image and clear the rest, scrolling the strip
   // to the checked one; false when its tile is not on this page.
   bool updateSelection(const std::wstring& currentPath)
   {
      bool found {false};

      for (const auto& child : m_ui.strip.Children()) {
         const auto tile = child.try_as<ToggleButton>();
         if (tile && tile.Tag())
            found |= checkTile(tile, currentPath);
      }

      return found;
   }

   // Tick the tile exactly when it carries 'currentPath' (clearing the others
   // on the way) and scroll it into view; true marks the hit.
   bool checkTile(const ToggleButton& tile, const std::wstring& currentPath)
   {
      const bool isCurrent {samePath(std::wstring {unbox_value<hstring>(tile.Tag())}, currentPath)};
      tile.IsChecked(isCurrent);

      if (isCurrent)
         scrollToTile(tile);

      return isCurrent;
   }

   // A tile built or revealed this very tick has no valid layout yet, so a
   // synchronous bring-into-view reads dead geometry and silently no-ops. Re-post
   // it at low priority: by the time it runs, measure/arrange has happened and the
   // scroller's extent is real. Capture only the tile — never 'this' — so a folder
   // change or teardown before it runs cannot dangle.
   void scrollToTile(const ToggleButton& tile) const
   {
      m_ui.strip.DispatcherQueue().TryEnqueue(DispatcherQueuePriority::Low,
                                              [tile] { bringIntoView(tile); });
   }

   // A tile already fully on screen stays put; one offscreen scrolls just into view.
   static void bringIntoView(const ToggleButton& tile)
   {
      BringIntoViewOptions options;
      options.AnimationDesired(false);
      tile.StartBringIntoView(options);
   }

   // One reel tile: a checkable button carrying its file path (Tag) and showing a
   // thumbnail above its name; a click reports the open request.
   UIElement makeThumb(const std::wstring& path)
   {
      ToggleButton tile;
      tile.Style(thumbStyle());
      tile.Tag(box_value(hstring {path}));
      tile.Content(makeContent(path));
      tile.Click({this, &Impl::onTileClick});
      return tile;
   }

   void onTileClick(const IInspectable& sender, const RoutedEventArgs&)
   {
      const auto tile = sender.try_as<ToggleButton>();
      if (!tile || !tile.Tag() || !m_onActivated)
         return;

      m_onActivated(std::wstring {unbox_value<hstring>(tile.Tag())});
   }

   UIElement makeContent(const std::wstring& path)
   {
      StackPanel panel;
      panel.Spacing(6.0);
      panel.Children().Append(makeFrame(path));
      panel.Children().Append(makeCaption(path));
      return panel;
   }

   // The framed preview: a rounded, dark backdrop with the (aspect-preserved)
   // thumbnail bitmap centred in it.
   UIElement makeFrame(const std::wstring& path)
   {
      Border frame;
      frame.CornerRadius(CornerRadius {6, 6, 6, 6});
      frame.Background(themeBrush(L"AbyssBrush"));

      Image image;
      image.Stretch(Stretch::Uniform);
      image.Height(k_thumbHeight);
      image.Source(thumbnailFor(path));
      frame.Child(image);
      return frame;
   }

   WriteableBitmap thumbnailFor(const std::wstring& path)
   {
      if (const auto cached = m_thumbCache.find(path); cached != m_thumbCache.end())
         return cached->second;

      if (m_thumbCache.size() >= k_thumbCacheCap)
         m_thumbCache.clear();

      return m_thumbCache.emplace(path, decodeThumb(path)).first->second;
   }

   WriteableBitmap decodeThumb(const std::wstring& path) const
   {
      return anka::GUI::Presentation::BitmapConversion::toWriteableBitmap(
         OWicImageThumbnailer::getInstance().thumbnail(path, k_thumbEdge));
   }

   UIElement makeCaption(const std::wstring& path)
   {
      TextBlock caption;
      caption.Text(hstring {std::filesystem::path {path}.filename().wstring()});
      caption.FontFamily(FontFamily {L"Consolas"});
      caption.FontSize(10.0);
      caption.Foreground(themeBrush(L"TextBrush"));
      caption.TextTrimming(TextTrimming::CharacterEllipsis);
      caption.HorizontalAlignment(HorizontalAlignment::Center);
      return caption;
   }

   // Resolve a brush / the reel-tile style from the host's themed resources, so
   // code-built tiles share the exact palette the XAML uses.
   Brush themeBrush(const hstring& key) const
   {
      return m_ui.resourceHost.Resources().Lookup(box_value(key)).as<Brush>();
   }

   Style thumbStyle(void) const
   {
      return m_ui.resourceHost.Resources().Lookup(box_value(hstring {L"ReelThumbStyle"})).as<Style>();
   }
};

// -----------------------------------------------------------------------------
// OFolderReel Implementation
// -----------------------------------------------------------------------------
OFolderReel::OFolderReel(std::vector<std::wstring> supportedFormats) :
   m_pImpl {std::make_unique<Impl>(std::move(supportedFormats))}
{}
// -----------------------------------------------------------------------------
OFolderReel::~OFolderReel() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OFolderReel::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OFolderReel::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OFolderReel::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OFolderReel::attach(const ReelControls& controls)
{
   m_pImpl->attach(controls);
}
// -----------------------------------------------------------------------------
void OFolderReel::show(const std::wstring& imagePath)
{
   m_pImpl->show(imagePath);
}
// -----------------------------------------------------------------------------
void OFolderReel::onImageActivated(ImageActivatedCallback callback)
{
   m_pImpl->m_onActivated = std::move(callback);
}
// -----------------------------------------------------------------------------
void OFolderReel::onExpandRequested(ExpandRequestedCallback callback)
{
   m_pImpl->m_onExpand = std::move(callback);
}
// -----------------------------------------------------------------------------
