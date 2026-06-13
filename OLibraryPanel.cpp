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
#include "OLibraryPanel.hpp"

#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"
#include "framework/Core/Paging/Pager.hpp"
#include "framework/Graphics/Image/OWicImageThumbnailer.hpp"
#include "framework/GUI/Menu/OMenuFactory.hpp"
#include "framework/GUI/Presentation/BitmapConversion.hpp"

#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Foundation.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <format>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace talay::GUI::Library;
using namespace talay::Library;

using anka::Core::Object::ObjectIdentity;
using anka::Core::Localization::OLocalizationService;
using anka::Core::Paging::Pager;
using anka::Graphics::Image::OWicImageThumbnailer;
using anka::GUI::Menu::MenuCommand;
using anka::GUI::Menu::menuSeparator;
using anka::GUI::Menu::OMenuFactory;

namespace BitmapConversion = anka::GUI::Presentation::BitmapConversion;

using winrt::hstring;
using winrt::box_value;
using winrt::unbox_value;
using winrt::Windows::Foundation::DateTime;
using winrt::Windows::Foundation::IInspectable;

using winrt::Microsoft::UI::Dispatching::DispatcherQueueTimer;
using winrt::Microsoft::UI::Xaml::FrameworkElement;
using winrt::Microsoft::UI::Xaml::GridLength;
using winrt::Microsoft::UI::Xaml::GridUnitType;
using winrt::Microsoft::UI::Xaml::HorizontalAlignment;
using winrt::Microsoft::UI::Xaml::RoutedEventArgs;
using winrt::Microsoft::UI::Xaml::Style;
using winrt::Microsoft::UI::Xaml::TextTrimming;
using winrt::Microsoft::UI::Xaml::UIElement;
using winrt::Microsoft::UI::Xaml::VerticalAlignment;
using winrt::Microsoft::UI::Xaml::Visibility;
using winrt::Microsoft::UI::Xaml::Controls::Border;
using winrt::Microsoft::UI::Xaml::Controls::Button;
using winrt::Microsoft::UI::Xaml::Controls::CheckBox;
using winrt::Microsoft::UI::Xaml::Controls::ColumnDefinition;
using winrt::Microsoft::UI::Xaml::Controls::ComboBoxItem;
using winrt::Microsoft::UI::Xaml::Controls::FontIcon;
using winrt::Microsoft::UI::Xaml::Controls::Grid;
using winrt::Microsoft::UI::Xaml::Controls::Image;
using winrt::Microsoft::UI::Xaml::Controls::NumberBox;
using winrt::Microsoft::UI::Xaml::Controls::Orientation;
using winrt::Microsoft::UI::Xaml::Controls::StackPanel;
using winrt::Microsoft::UI::Xaml::Controls::TextBlock;
using winrt::Microsoft::UI::Xaml::Controls::ToolTipService;
using winrt::Microsoft::UI::Xaml::CornerRadius;
using winrt::Microsoft::UI::Xaml::Media::Brush;
using winrt::Microsoft::UI::Xaml::Media::FontFamily;
using winrt::Microsoft::UI::Xaml::Media::Stretch;
using winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap;


namespace
{
   constexpr int           k_debounceMs      {250};
   constexpr std::int64_t  k_secondsPerDay   {86'400};
   constexpr std::uint32_t k_thumbEdge       {200};     // decoded longest side, px
   constexpr double        k_tileWidth       {150.0};
   constexpr double        k_tileImageHeight {110.0};
   constexpr double        k_starSize        {11.0};

   // Segoe MDL2 Assets glyphs for the entry context menu; open and convert
   // reuse the File menu's glyphs so the actions read as the same ones.
   constexpr wchar_t k_openGlyph[]       {L""};   // OpenFolderHorizontal
   constexpr wchar_t k_removeGlyph[]     {L""};   // Remove
   constexpr wchar_t k_convertGlyph[]    {L""};   // Switch
   constexpr wchar_t k_unfavoriteGlyph[] {L""};   // Unfavorite

   // One entry per LibPageSize combo item, in XAML order — the combo's
   // SelectedIndex indexes straight into this.
   constexpr std::array<std::size_t, 4> k_pageSizes        {25, 50, 100, 200};
   constexpr std::size_t                k_defaultSizeIndex {1};

   std::wstring toUpper(std::wstring text)
   {
      std::transform(text.begin(), text.end(), text.begin(),
                     [](wchar_t character) { return static_cast<wchar_t>(std::towupper(character)); });
      return text;
   }

   std::wstring joined(const std::vector<std::wstring>& values)
   {
      std::wstring text;
      for (const auto& value : values)
         text += text.empty() ? value : L", " + value;

      return text;
   }

   // The localized count patterns ("{} results") arrive at run time, so this
   // goes through vformat rather than a compile-time checked format string.
   std::wstring formatCount(const std::wstring& pattern, std::size_t count)
   {
      return std::vformat(pattern, std::make_wformat_args(count));
   }

   bool isChecked(const CheckBox& box)
   {
      const auto state = box.IsChecked();
      return state && state.Value();
   }

   std::int64_t unixSecondsFrom(const DateTime& moment)
   {
      return static_cast<std::int64_t>(winrt::clock::to_time_t(moment));
   }

   FILETIME fromUnixSeconds(std::int64_t seconds)
   {
      ULARGE_INTEGER ticks {};
      ticks.QuadPart = static_cast<std::uint64_t>(seconds + 11'644'473'600LL) * 10'000'000ULL;

      FILETIME fileTime {};
      fileTime.dwLowDateTime  = ticks.LowPart;
      fileTime.dwHighDateTime = ticks.HighPart;
      return fileTime;
   }

   // -------------------------------------------------------------------------
   // Row / tile texts — presentation of raw entry numbers
   // -------------------------------------------------------------------------

   std::wstring dimensionsText(const LibraryEntry& entry)
   {
      if (entry.width == 0 || entry.height == 0)
         return L"-";

      return std::format(L"{} x {}", entry.width, entry.height);
   }

   std::wstring sizeText(const LibraryEntry& entry)
   {
      return std::format(L"{} KB", entry.fileSizeBytes / 1024u);
   }

   std::wstring dateText(std::int64_t unixSeconds)
   {
      FILETIME utc = fromUnixSeconds(unixSeconds);
      FILETIME local {};
      SYSTEMTIME civil {};
      if (unixSeconds == 0 || !FileTimeToLocalFileTime(&utc, &local) || !FileTimeToSystemTime(&local, &civil))
         return L"-";

      return std::format(L"{:04}-{:02}-{:02}", civil.wYear, civil.wMonth, civil.wDay);
   }

   std::wstring leafName(const std::wstring& folder)
   {
      const std::wstring leaf = std::filesystem::path {folder}.filename().wstring();
      return leaf.empty() ? folder : leaf;
   }

   // -------------------------------------------------------------------------
   // Sort strategies — one comparator per sort-mode combo entry
   // -------------------------------------------------------------------------

   using Comparator = bool (*)(const LibraryEntry&, const LibraryEntry&);

   bool byDate(const LibraryEntry& left, const LibraryEntry& right)
   {
      return left.modifiedUtc > right.modifiedUtc;
   }

   bool byName(const LibraryEntry& left, const LibraryEntry& right)
   {
      return toUpper(left.fileName) < toUpper(right.fileName);
   }

   bool byFormat(const LibraryEntry& left, const LibraryEntry& right)
   {
      return left.format == right.format ? byName(left, right) : left.format < right.format;
   }

   bool bySize(const LibraryEntry& left, const LibraryEntry& right)
   {
      return left.fileSizeBytes > right.fileSizeBytes;
   }

   Comparator comparatorFor(int sortIndex)
   {
      switch (sortIndex) {
         case 1:  return &byName;
         case 2:  return &byFormat;
         case 3:  return &bySize;
         default: return &byDate;
      }
   }
}


// -----------------------------------------------------------------------------
// OLibraryPanel::Impl Definition
// -----------------------------------------------------------------------------
struct OLibraryPanel::Impl
{
   ObjectIdentity m_identity {L"LibraryPanelObject"};

   IImageLibrary&              m_library;
   LibraryPanelControls        m_ui;
   OpenRequestedCallback       m_onOpen;
   RemoveRequestedCallback     m_onRemove;
   ConvertRequestedCallback    m_onConvert;
   UnfavoriteRequestedCallback m_onUnfavorite;

   DispatcherQueueTimer m_debounce {nullptr};

   // True while code rewrites controls (reset, dropdown rebuilds); the change
   // handlers see it and stay quiet, so one user action means one query.
   bool m_muted {false};

   std::vector<LibraryEntry> m_results;

   // Slices m_results into pages; only the current page is ever rendered.
   Pager m_pager;

   // Thumbnails decoded once per path and kept for the session; re-filtering
   // rebuilds tiles but never re-decodes.
   std::unordered_map<std::wstring, WriteableBitmap> m_thumbCache;

   // Menu labels resolved once — the locale is fixed for the process, so
   // per-row menu construction never re-hits the resource map.
   const std::wstring m_openLabel       {OLocalizationService::getInstance().text(L"LibMenuOpen")};
   const std::wstring m_convertLabel    {OLocalizationService::getInstance().text(L"LibMenuConvert")};
   const std::wstring m_unfavoriteLabel {OLocalizationService::getInstance().text(L"LibMenuUnfavorite")};
   const std::wstring m_removeLabel     {OLocalizationService::getInstance().text(L"LibMenuRemove")};

   explicit Impl(IImageLibrary& library) :
      m_library {library}
   {}

   ~Impl() noexcept
   {
      if (m_debounce)
         m_debounce.Stop();

      m_library.onChanged({});
   }

   //
   // Wiring
   //

   void attach(const LibraryPanelControls& controls)
   {
      m_ui = controls;
      createDebounceTimer();
      wireFilterEvents();
      wireViewEvents();
      wirePagerEvents();
      m_pager.setPageSize(selectedPageSize());
      m_library.onChanged([this] { onLibraryChanged(); });
      applyDisplayMode();
      refreshAll();
   }

   void createDebounceTimer(void)
   {
      m_debounce = m_ui.nameBox.DispatcherQueue().CreateTimer();
      m_debounce.Interval(std::chrono::milliseconds {k_debounceMs});
      m_debounce.IsRepeating(false);
      m_debounce.Tick([this](auto&&, auto&&) { refreshResults(); });
   }

   void wireFilterEvents(void)
   {
      m_ui.nameBox.TextChanged([this](auto&&, auto&&) { scheduleRefresh(); });
      wireRangeEvents();
      wireDateEvents();
      wireCheckBox(m_ui.stills);
      wireCheckBox(m_ui.animated);
      wireCheckBox(m_ui.favoritesOnly);
   }

   void wireRangeEvents(void)
   {
      const auto onValue = [this](auto&&, auto&&) { scheduleRefresh(); };
      m_ui.sizeMin.ValueChanged(onValue);
      m_ui.sizeMax.ValueChanged(onValue);
      m_ui.widthMin.ValueChanged(onValue);
      m_ui.widthMax.ValueChanged(onValue);
      m_ui.heightMin.ValueChanged(onValue);
      m_ui.heightMax.ValueChanged(onValue);
   }

   void wireDateEvents(void)
   {
      const auto onDate = [this](auto&&, auto&&) { refreshResults(); };
      m_ui.dateFrom.DateChanged(onDate);
      m_ui.dateTo.DateChanged(onDate);
   }

   void wireCheckBox(const CheckBox& box)
   {
      box.Checked([this](auto&&, auto&&) { refreshResults(); });
      box.Unchecked([this](auto&&, auto&&) { refreshResults(); });
   }

   void wireViewEvents(void)
   {
      m_ui.folderBox.SelectionChanged([this](auto&&, auto&&) { refreshResults(); });
      m_ui.sortMode.SelectionChanged([this](auto&&, auto&&) { refreshResults(); });
      m_ui.displayMode.SelectionChanged([this](auto&&, auto&&) { onDisplayModeChanged(); });
   }

   void wirePagerEvents(void)
   {
      m_ui.pagePrev.Click([this](auto&&, auto&&) { onPreviousPage(); });
      m_ui.pageNext.Click([this](auto&&, auto&&) { onNextPage(); });
      m_ui.pageSize.SelectionChanged([this](auto&&, auto&&) { onPageSizeChanged(); });
   }

   template <typename Action>
   void withControlsMuted(Action action)
   {
      m_muted = true;
      action();
      m_muted = false;
   }

   //
   // Refresh pipeline: controls -> LibraryFilter -> query -> render
   //

   void scheduleRefresh(void)
   {
      if (m_muted)
         return;

      m_debounce.Stop();
      m_debounce.Start();
   }

   // Filter or sort edits change what the list means, so they jump back to
   // the first page; library changes (favorite toggles, removals, imports)
   // re-run the same view and only pull the page back if the list shrank
   // under it. The mute guard matters here: programmatic dropdown rebuilds
   // raise SelectionChanged into this path and must not reset the page.
   void refreshResults(void)
   {
      if (m_muted)
         return;

      m_pager.first();
      requery();
   }

   void requery(void)
   {
      if (m_muted || !m_ui.tableRows)
         return;

      m_results = m_library.query(buildFilter());
      std::sort(m_results.begin(), m_results.end(), comparatorFor(m_ui.sortMode.SelectedIndex()));
      m_pager.setItemCount(m_results.size());
      renderActive();
      updateCounters();
   }

   void refreshAll(void)
   {
      withControlsMuted([this] {
         rebuildFormatList();
         rebuildFolderList();
      });

      requery();
   }

   void onLibraryChanged(void)
   {
      refreshAll();
   }

   //
   // Filter assembly
   //

   LibraryFilter buildFilter(void)
   {
      LibraryFilter filter;
      fillTextCriteria(filter);
      fillRangeCriteria(filter);
      fillFlagCriteria(filter);
      return filter;
   }

   void fillTextCriteria(LibraryFilter& filter)
   {
      filter.nameContains = std::wstring {m_ui.nameBox.Text()};
      filter.formats      = checkedFormats();
      filter.folder       = selectedFolder();
   }

   void fillRangeCriteria(LibraryFilter& filter)
   {
      filter.sizeKb      = {boxValue<std::uint64_t>(m_ui.sizeMin), boxValue<std::uint64_t>(m_ui.sizeMax)};
      filter.width       = {boxValue<std::uint32_t>(m_ui.widthMin), boxValue<std::uint32_t>(m_ui.widthMax)};
      filter.height      = {boxValue<std::uint32_t>(m_ui.heightMin), boxValue<std::uint32_t>(m_ui.heightMax)};
      filter.modifiedUtc = datePeriod();
   }

   void fillFlagCriteria(LibraryFilter& filter)
   {
      filter.favoritesOnly   = isChecked(m_ui.favoritesOnly);
      filter.includeStills   = isChecked(m_ui.stills);
      filter.includeAnimated = isChecked(m_ui.animated);
   }

   // An empty NumberBox reports NaN — that bound is simply not set.
   template <typename Value>
   static std::optional<Value> boxValue(const NumberBox& box)
   {
      const double value = box.Value();
      if (std::isnan(value) || value < 0.0)
         return std::nullopt;

      return static_cast<Value>(value);
   }

   // The end picker selects a day, so the bound runs to that day's last second.
   Range<std::int64_t> datePeriod(void)
   {
      Range<std::int64_t> period;
      if (const auto from = m_ui.dateFrom.Date())
         period.minimum = unixSecondsFrom(from.Value());

      if (const auto to = m_ui.dateTo.Date())
         period.maximum = unixSecondsFrom(to.Value()) + k_secondsPerDay - 1;

      return period;
   }

   std::vector<std::wstring> checkedFormats(void) const
   {
      std::vector<std::wstring> formats;
      for (const auto& child : m_ui.formatList.Children()) {
         const auto box = child.try_as<CheckBox>();
         if (box && isChecked(box))
            formats.push_back(std::wstring {unbox_value<hstring>(box.Tag())});
      }

      return formats;
   }

   std::wstring selectedFolder(void) const
   {
      if (m_ui.folderBox.SelectedIndex() <= 0)
         return {};

      const auto item = m_ui.folderBox.SelectedItem().try_as<ComboBoxItem>();
      return item && item.Tag() ? std::wstring {unbox_value<hstring>(item.Tag())} : std::wstring {};
   }

   //
   // Dynamic dropdowns — formats / folders actually present in the library
   //

   void rebuildFormatList(void)
   {
      const auto selected = checkedFormats();

      auto boxes = m_ui.formatList.Children();
      boxes.Clear();
      for (const auto& format : m_library.availableFormats())
         boxes.Append(makeFormatCheckBox(format, contains(selected, format)));

      updateFormatSummary();
   }

   static bool contains(const std::vector<std::wstring>& values, const std::wstring& value)
   {
      return std::find(values.begin(), values.end(), value) != values.end();
   }

   CheckBox makeFormatCheckBox(const std::wstring& format, bool checked)
   {
      CheckBox box;
      box.Content(box_value(hstring {format}));
      box.Tag(box_value(hstring {format}));
      box.MinWidth(0.0);
      box.IsChecked(checked);
      box.Checked({this, &Impl::onFormatToggled});
      box.Unchecked({this, &Impl::onFormatToggled});
      return box;
   }

   void onFormatToggled(const IInspectable&, const RoutedEventArgs&)
   {
      if (m_muted)
         return;

      updateFormatSummary();
      refreshResults();
   }

   void updateFormatSummary(void)
   {
      m_ui.formatSummary.Text(hstring {formatSummaryText(checkedFormats())});
   }

   // "All" when nothing is ticked, the names while they fit, a count beyond.
   std::wstring formatSummaryText(const std::vector<std::wstring>& formats) const
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};
      if (formats.empty())
         return loc.text(L"LibFormatAll");

      if (formats.size() <= 2)
         return joined(formats);

      return formatCount(loc.text(L"LibFormatCount"), formats.size());
   }

   void rebuildFolderList(void)
   {
      const std::wstring previous = selectedFolder();
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      auto items = m_ui.folderBox.Items();
      items.Clear();
      items.Append(makeFolderItem(loc.text(L"LibFolderAll"), L""));
      for (const auto& folder : m_library.availableFolders())
         items.Append(makeFolderItem(leafName(folder), folder));

      reselectFolder(previous);
   }

   ComboBoxItem makeFolderItem(const std::wstring& label, const std::wstring& fullPath)
   {
      ComboBoxItem item;
      item.Content(box_value(hstring {label}));
      item.Tag(box_value(hstring {fullPath}));
      if (!fullPath.empty())
         ToolTipService::SetToolTip(item, box_value(hstring {fullPath}));

      return item;
   }

   void reselectFolder(const std::wstring& previous)
   {
      const auto items = m_ui.folderBox.Items();
      for (std::uint32_t index = 0; index < items.Size(); ++index) {
         if (folderTagAt(index) == previous) {
            m_ui.folderBox.SelectedIndex(static_cast<std::int32_t>(index));
            return;
         }
      }

      m_ui.folderBox.SelectedIndex(0);
   }

   std::wstring folderTagAt(std::uint32_t index) const
   {
      const auto item = m_ui.folderBox.Items().GetAt(index).try_as<ComboBoxItem>();
      return item && item.Tag() ? std::wstring {unbox_value<hstring>(item.Tag())} : std::wstring {};
   }

   //
   // Result rendering — table and thumbnail views
   //

   // Item order is the XAML combo's: 0 = thumbnails (the default view, and
   // what an unset combo falls back to), 1 = table.
   bool tableModeActive(void) const
   {
      return m_ui.displayMode.SelectedIndex() == 1;
   }

   void onDisplayModeChanged(void)
   {
      if (!m_muted)
         applyDisplayMode();
   }

   void applyDisplayMode(void)
   {
      const bool table = tableModeActive();
      m_ui.tableView.Visibility(table ? Visibility::Visible : Visibility::Collapsed);
      m_ui.thumbGrid.Visibility(table ? Visibility::Collapsed : Visibility::Visible);
      renderActive();
   }

   // Only the visible view is (re)built, and only from the current page of
   // the cached result list; switching views renders the other on demand.
   void renderActive(void)
   {
      if (tableModeActive())
         renderTable();
      else
         renderThumbnails();

      m_ui.emptyLabel.Visibility(m_results.empty() ? Visibility::Visible : Visibility::Collapsed);
      updatePagerBar();
   }

   void updateCounters(void)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};
      m_ui.itemCount.Text(hstring {formatCount(loc.text(L"LibItemsCount"), m_library.entryCount())});
      m_ui.resultCount.Text(hstring {formatCount(loc.text(L"LibResultsCount"), m_results.size())});
   }

   //
   // Paging — the pager owns the arithmetic, this owns the controls
   //

   std::span<const LibraryEntry> pageEntries(void) const
   {
      return std::span<const LibraryEntry> {m_results}
         .subspan(m_pager.firstItem(), m_pager.itemsOnPage());
   }

   void updatePagerBar(void)
   {
      m_ui.pageLabel.Text(hstring {pageLabelText()});
      m_ui.pagePrev.IsEnabled(m_pager.hasPrevious());
      m_ui.pageNext.IsEnabled(m_pager.hasNext());
   }

   std::wstring pageLabelText(void) const
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};
      const std::size_t number {m_pager.pageNumber()};
      const std::size_t count  {m_pager.pageCount()};
      return std::vformat(loc.text(L"LibPageLabel"), std::make_wformat_args(number, count));
   }

   // Turning a page re-renders the cached results; no requery happens.
   void onPreviousPage(void)
   {
      if (m_pager.previous())
         renderActive();
   }

   void onNextPage(void)
   {
      if (m_pager.next())
         renderActive();
   }

   void onPageSizeChanged(void)
   {
      if (m_muted)
         return;

      m_pager.setPageSize(selectedPageSize());
      renderActive();
   }

   std::size_t selectedPageSize(void) const
   {
      const int index {m_ui.pageSize.SelectedIndex()};
      if (index < 0 || index >= static_cast<int>(k_pageSizes.size()))
         return k_pageSizes[k_defaultSizeIndex];

      return k_pageSizes[static_cast<std::size_t>(index)];
   }

   void renderTable(void)
   {
      auto rows = m_ui.tableRows.Children();
      rows.Clear();
      for (const auto& entry : pageEntries())
         rows.Append(makeRow(entry));
   }

   UIElement makeRow(const LibraryEntry& entry)
   {
      Button row;
      row.Style(themeStyle(L"LibRowStyle"));
      row.Tag(box_value(hstring {entry.filePath}));
      row.Content(makeRowCells(entry));
      row.Click({this, &Impl::onEntryClick});
      attachEntryMenu(row, entry);
      return row;
   }

   UIElement makeRowCells(const LibraryEntry& entry)
   {
      Grid cells;
      appendRowColumns(cells);
      addCell(cells, 0, makeStarCell(entry));
      addCell(cells, 1, makeTextCell(entry.fileName, L"FoamBrush"));
      addCell(cells, 2, makeTextCell(entry.format, L"TextBrush"));
      addCell(cells, 3, makeTextCell(dimensionsText(entry), L"TextBrush"));
      addCell(cells, 4, makeTextCell(sizeText(entry), L"TextBrush"));
      addCell(cells, 5, makeTextCell(dateText(entry.modifiedUtc), L"GoldBrush"));
      return cells;
   }

   // Column widths mirror the XAML header row, so cells line up under it.
   void appendRowColumns(const Grid& grid)
   {
      appendColumn(grid, GridLength {30.0, GridUnitType::Pixel});
      appendColumn(grid, GridLength {1.0, GridUnitType::Star});
      appendColumn(grid, GridLength {70.0, GridUnitType::Pixel});
      appendColumn(grid, GridLength {104.0, GridUnitType::Pixel});
      appendColumn(grid, GridLength {80.0, GridUnitType::Pixel});
      appendColumn(grid, GridLength {96.0, GridUnitType::Pixel});
   }

   void appendColumn(const Grid& grid, const GridLength& width)
   {
      ColumnDefinition column;
      column.Width(width);
      grid.ColumnDefinitions().Append(column);
   }

   void addCell(const Grid& grid, std::int32_t column, const FrameworkElement& cell)
   {
      Grid::SetColumn(cell, column);
      grid.Children().Append(cell);
   }

   FrameworkElement makeStarCell(const LibraryEntry& entry)
   {
      if (!entry.isFavorite)
         return TextBlock {};

      auto star = makeStarIcon();
      star.HorizontalAlignment(HorizontalAlignment::Left);
      star.VerticalAlignment(VerticalAlignment::Center);
      return star;
   }

   FontIcon makeStarIcon(void)
   {
      FontIcon star;
      star.Glyph(hstring {L"\uE735"});   // FavoriteStarFill
      star.FontFamily(FontFamily {L"Segoe MDL2 Assets"});
      star.FontSize(k_starSize);
      star.Foreground(themeBrush(L"GoldBrush"));
      return star;
   }

   TextBlock makeTextCell(const std::wstring& text, const wchar_t* brushKey)
   {
      TextBlock cell;
      cell.Text(hstring {text});
      cell.FontFamily(FontFamily {L"Consolas"});
      cell.FontSize(11.5);
      cell.Foreground(themeBrush(brushKey));
      cell.VerticalAlignment(VerticalAlignment::Center);
      cell.TextTrimming(TextTrimming::CharacterEllipsis);
      return cell;
   }

   void onEntryClick(const IInspectable& sender, const RoutedEventArgs&)
   {
      const auto element = sender.try_as<FrameworkElement>();
      if (element && element.Tag())
         fireOpen(std::wstring {unbox_value<hstring>(element.Tag())});
   }

   //
   // Entry context menu — right-click offers open / convert plus, below a
   // separator, dropping the favorite mark (favorites only) and removal.
   // Every pick reports through the host callbacks (the panel neither opens
   // files nor mutates the library itself).
   //

   void attachEntryMenu(const Button& item, const LibraryEntry& entry) const
   {
      OMenuFactory::getInstance().attachContextMenu(item, entryCommands(entry));
   }

   std::vector<MenuCommand> entryCommands(const LibraryEntry& entry) const
   {
      std::vector<MenuCommand> commands {openCommand(entry.filePath), convertCommand(entry.filePath)};
      commands.push_back(menuSeparator());
      if (entry.isFavorite)
         commands.push_back(unfavoriteCommand(entry.filePath));

      commands.push_back(removeCommand(entry.filePath));
      return commands;
   }

   MenuCommand openCommand(const std::wstring& path) const
   {
      return {.label  = m_openLabel,
              .glyph  = k_openGlyph,
              .action = [this, path] { fireOpen(path); }};
   }

   MenuCommand convertCommand(const std::wstring& path) const
   {
      return {.label  = m_convertLabel,
              .glyph  = k_convertGlyph,
              .action = [this, path] { fireConvert(path); }};
   }

   MenuCommand unfavoriteCommand(const std::wstring& path) const
   {
      return {.label  = m_unfavoriteLabel,
              .glyph  = k_unfavoriteGlyph,
              .action = [this, path] { fireUnfavorite(path); }};
   }

   MenuCommand removeCommand(const std::wstring& path) const
   {
      return {.label  = m_removeLabel,
              .glyph  = k_removeGlyph,
              .action = [this, path] { fireRemove(path); }};
   }

   void fireOpen(const std::wstring& path) const
   {
      if (m_onOpen)
         m_onOpen(path);
   }

   void fireConvert(const std::wstring& path) const
   {
      if (m_onConvert)
         m_onConvert(path);
   }

   void fireUnfavorite(const std::wstring& path) const
   {
      if (m_onUnfavorite)
         m_onUnfavorite(path);
   }

   void fireRemove(const std::wstring& path) const
   {
      if (m_onRemove)
         m_onRemove(path);
   }

   void renderThumbnails(void)
   {
      auto tiles = m_ui.thumbGrid.Items();
      tiles.Clear();
      for (const auto& entry : pageEntries())
         tiles.Append(makeTile(entry));
   }

   UIElement makeTile(const LibraryEntry& entry)
   {
      Button tile;
      tile.Style(themeStyle(L"LibTileStyle"));
      tile.Tag(box_value(hstring {entry.filePath}));
      tile.Content(makeTileContent(entry));
      tile.Click({this, &Impl::onEntryClick});
      attachEntryMenu(tile, entry);
      return tile;
   }

   UIElement makeTileContent(const LibraryEntry& entry)
   {
      StackPanel panel;
      panel.Spacing(6.0);
      panel.Width(k_tileWidth);
      panel.Children().Append(makeTileFrame(entry));
      panel.Children().Append(makeTileCaption(entry));
      return panel;
   }

   UIElement makeTileFrame(const LibraryEntry& entry)
   {
      Border frame;
      frame.CornerRadius(CornerRadius {6, 6, 6, 6});
      frame.Background(themeBrush(L"AbyssBrush"));

      Image image;
      image.Stretch(Stretch::Uniform);
      image.Height(k_tileImageHeight);
      image.Source(thumbnailFor(entry.filePath));
      frame.Child(image);
      return frame;
   }

   UIElement makeTileCaption(const LibraryEntry& entry)
   {
      StackPanel caption;
      caption.Orientation(Orientation::Horizontal);
      caption.Spacing(5.0);
      caption.HorizontalAlignment(HorizontalAlignment::Center);
      if (entry.isFavorite)
         caption.Children().Append(makeStarIcon());

      caption.Children().Append(makeCaptionText(entry.fileName));
      return caption;
   }

   TextBlock makeCaptionText(const std::wstring& name)
   {
      TextBlock text;
      text.Text(hstring {name});
      text.FontFamily(FontFamily {L"Consolas"});
      text.FontSize(10.0);
      text.Foreground(themeBrush(L"TextBrush"));
      text.TextTrimming(TextTrimming::CharacterEllipsis);
      text.MaxWidth(k_tileWidth - 24.0);
      return text;
   }

   WriteableBitmap thumbnailFor(const std::wstring& path)
   {
      if (const auto cached = m_thumbCache.find(path); cached != m_thumbCache.end())
         return cached->second;

      const auto bitmap = BitmapConversion::toWriteableBitmap(
         OWicImageThumbnailer::getInstance().thumbnail(path, k_thumbEdge));
      m_thumbCache.emplace(path, bitmap);
      return bitmap;
   }

   //
   // Reset
   //

   void resetFilters(void)
   {
      withControlsMuted([this] {
         clearTextFilters();
         clearRangeFilters();
         clearFlagFilters();
      });

      refreshResults();
   }

   void clearTextFilters(void)
   {
      m_ui.nameBox.Text(L"");
      uncheckFormatBoxes();
      updateFormatSummary();
      m_ui.folderBox.SelectedIndex(0);
   }

   void uncheckFormatBoxes(void)
   {
      for (const auto& child : m_ui.formatList.Children()) {
         if (const auto box = child.try_as<CheckBox>())
            box.IsChecked(false);
      }
   }

   void clearRangeFilters(void)
   {
      const double cleared = std::nan("");   // NaN empties a NumberBox
      m_ui.sizeMin.Value(cleared);
      m_ui.sizeMax.Value(cleared);
      m_ui.widthMin.Value(cleared);
      m_ui.widthMax.Value(cleared);
      m_ui.heightMin.Value(cleared);
      m_ui.heightMax.Value(cleared);
      m_ui.dateFrom.Date(nullptr);
      m_ui.dateTo.Date(nullptr);
   }

   void clearFlagFilters(void)
   {
      m_ui.stills.IsChecked(true);
      m_ui.animated.IsChecked(true);
      m_ui.favoritesOnly.IsChecked(false);
   }

   //
   // Themed resources, borrowed from the bound host
   //

   Brush themeBrush(const wchar_t* key) const
   {
      return m_ui.resourceHost.Resources().Lookup(box_value(hstring {key})).as<Brush>();
   }

   Style themeStyle(const wchar_t* key) const
   {
      return m_ui.resourceHost.Resources().Lookup(box_value(hstring {key})).as<Style>();
   }
};

// -----------------------------------------------------------------------------
// OLibraryPanel Implementation
// -----------------------------------------------------------------------------
OLibraryPanel::OLibraryPanel(IImageLibrary& library) :
   m_pImpl {std::make_unique<Impl>(library)}
{}
// -----------------------------------------------------------------------------
OLibraryPanel::~OLibraryPanel() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OLibraryPanel::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OLibraryPanel::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OLibraryPanel::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OLibraryPanel::attach(const LibraryPanelControls& controls)
{
   m_pImpl->attach(controls);
}
// -----------------------------------------------------------------------------
void OLibraryPanel::refresh(void)
{
   m_pImpl->refreshAll();
}
// -----------------------------------------------------------------------------
void OLibraryPanel::resetFilters(void)
{
   m_pImpl->resetFilters();
}
// -----------------------------------------------------------------------------
void OLibraryPanel::onOpenRequested(OpenRequestedCallback callback)
{
   m_pImpl->m_onOpen = std::move(callback);
}
// -----------------------------------------------------------------------------
void OLibraryPanel::onRemoveRequested(RemoveRequestedCallback callback)
{
   m_pImpl->m_onRemove = std::move(callback);
}
// -----------------------------------------------------------------------------
void OLibraryPanel::onConvertRequested(ConvertRequestedCallback callback)
{
   m_pImpl->m_onConvert = std::move(callback);
}
// -----------------------------------------------------------------------------
void OLibraryPanel::onUnfavoriteRequested(UnfavoriteRequestedCallback callback)
{
   m_pImpl->m_onUnfavorite = std::move(callback);
}
// -----------------------------------------------------------------------------
