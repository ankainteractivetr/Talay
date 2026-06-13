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
#include "ORecentFilesMenu.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <filesystem>
#include <string>
#include <utility>

using namespace anka::GUI::Menu;
using anka::Core::Object::ObjectIdentity;
using anka::Core::RecentFiles::IRecentFilesTracker;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;


// -----------------------------------------------------------------------------
// ORecentFilesMenu::Impl Definition
// -----------------------------------------------------------------------------
struct ORecentFilesMenu::Impl
{
   ObjectIdentity m_identity {L"RecentFilesMenuObject"};

   IRecentFilesTracker& m_tracker;

   MenuFlyoutSubItem  m_submenu     {nullptr};
   MenuFlyoutItemBase m_placeholder {nullptr};   // the markup "no files yet" row

   RecentFileOpenCallback m_onOpenRequested;

   explicit Impl(IRecentFilesTracker& tracker) :
      m_tracker {tracker}
   {}

   void attach(const MenuFlyoutSubItem& submenu)
   {
      m_submenu     = submenu;
      m_placeholder = submenu.Items().GetAt(0);

      m_tracker.onChanged([this] { rebuild(); });
      rebuild();
   }

   void rebuild(void)
   {
      const auto paths = m_tracker.files();

      clearEntries();
      m_placeholder.Visibility(paths.empty() ? Visibility::Visible : Visibility::Collapsed);

      for (const auto& path : paths)
         m_submenu.Items().Append(makeEntry(path));
   }

   // Everything after the placeholder is ours to rebuild.
   void clearEntries(void)
   {
      while (m_submenu.Items().Size() > 1)
         m_submenu.Items().RemoveAtEnd();
   }

   // File name as the label (the menu stays narrow), full path as the
   // tooltip (same-named files in different folders stay tellable apart).
   MenuFlyoutItem makeEntry(const std::wstring& path)
   {
      MenuFlyoutItem item;
      item.Text(hstring {std::filesystem::path {path}.filename().wstring()});
      ToolTipService::SetToolTip(item, box_value(hstring {path}));

      item.Click([this, path](auto&&, auto&&) { requestOpen(path); });
      return item;
   }

   void requestOpen(const std::wstring& path)
   {
      if (m_onOpenRequested)
         m_onOpenRequested(path);
   }
};

// -----------------------------------------------------------------------------
// ORecentFilesMenu Implementation
// -----------------------------------------------------------------------------
ORecentFilesMenu::ORecentFilesMenu(IRecentFilesTracker& tracker) :
   m_pImpl {std::make_unique<Impl>(tracker)}
{}
// -----------------------------------------------------------------------------
ORecentFilesMenu::~ORecentFilesMenu() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& ORecentFilesMenu::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void ORecentFilesMenu::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t ORecentFilesMenu::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void ORecentFilesMenu::attach(const MenuFlyoutSubItem& submenu)
{
   m_pImpl->attach(submenu);
}
// -----------------------------------------------------------------------------
void ORecentFilesMenu::onOpenRequested(RecentFileOpenCallback callback)
{
   m_pImpl->m_onOpenRequested = std::move(callback);
}
// -----------------------------------------------------------------------------
