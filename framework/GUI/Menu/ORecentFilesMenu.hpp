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

#ifndef ANKA_GUI_MENU_ORECENTFILESMENU_HPP
#define ANKA_GUI_MENU_ORECENTFILESMENU_HPP

#include "IRecentFilesMenu.hpp"
#include "framework/Core/RecentFiles/IRecentFilesTracker.hpp"

#include <memory>

namespace anka::GUI::Menu
{
   /*
   *
   * Concrete Recent Files Menu Class
   *
   * Observes the tracker (Observer): each change clears the submenu back to
   * its placeholder and appends one item per remembered path, labelled with
   * the file name and tooltipped with the full path. Clicks surface through
   * the injected open callback — pure presentation, no file or list policy
   * of its own. The host window owns one instance per submenu (not a
   * singleton); the tracker must outlive it.
   *
   */
   class ORecentFilesMenu final : public IRecentFilesMenu
   {
      public:

         explicit ORecentFilesMenu(anka::Core::RecentFiles::IRecentFilesTracker& tracker);

         ORecentFilesMenu(const ORecentFilesMenu& menu) = delete;
         ORecentFilesMenu(ORecentFilesMenu&& menu) noexcept = delete;

         ~ORecentFilesMenu() noexcept override;

         ORecentFilesMenu& operator=(const ORecentFilesMenu& menu) = delete;
         ORecentFilesMenu& operator=(ORecentFilesMenu&& menu) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IRecentFilesMenu overrides
         //

         void attach(const winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSubItem& submenu) override;

         void onOpenRequested(RecentFileOpenCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_MENU_ORECENTFILESMENU_HPP
