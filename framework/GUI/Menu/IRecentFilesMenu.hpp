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

#ifndef ANKA_GUI_MENU_IRECENTFILESMENU_HPP
#define ANKA_GUI_MENU_IRECENTFILESMENU_HPP

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "framework/Core/Object/IObject.hpp"

#include <functional>
#include <string>

namespace anka::GUI::Menu
{
   // Fired when a recent entry is clicked, carrying the file's full path so
   // the host can open it. The menu never opens files itself — it only
   // reports the request.
   using RecentFileOpenCallback = std::function<void(const std::wstring& /*path*/)>;

   /*
   * Abstract Recent Files Menu Interface
   *
   * Keeps a "Recent Files" submenu mirroring a recent-files tracker: one
   * clickable entry per remembered path, newest first, rebuilt whenever the
   * tracker changes. The host binds the submenu once via attach() and
   * subscribes to the open callback; everything WinUI about building the
   * entries stays hidden behind the implementation, so the host depends
   * only on this abstraction.
   *
   */
   class IRecentFilesMenu : public anka::Core::Object::IObject
   {
      public:

         virtual ~IRecentFilesMenu() noexcept override = default;

         // Bind the submenu to keep filled. Its first markup child must be
         // the (host-localized) "no files yet" placeholder item — shown
         // while the list is empty, hidden once entries exist.
         virtual void attach(const winrt::Microsoft::UI::Xaml::Controls::MenuFlyoutSubItem& submenu) = 0;

         virtual void onOpenRequested(RecentFileOpenCallback callback) = 0;
   };
}

#endif // ANKA_GUI_MENU_IRECENTFILESMENU_HPP
