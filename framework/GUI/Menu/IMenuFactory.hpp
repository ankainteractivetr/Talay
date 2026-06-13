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

#ifndef ANKA_GUI_MENU_IMENUFACTORY_HPP
#define ANKA_GUI_MENU_IMENUFACTORY_HPP

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

#include "framework/Core/Object/IObject.hpp"
#include "MenuCommand.hpp"

#include <span>

namespace anka::GUI::Menu
{
   /*
   * Abstract Menu Factory Interface
   *
   * Builds WinUI menu flyouts from plain command descriptors (a Factory
   * Method over the Command pattern): a caller lists what the menu offers
   * (label, icon, behaviour) and never touches a MenuFlyoutItem or wires a
   * Click handler. attachContextMenu() additionally binds the built menu as
   * an element's right-click flyout, so most call sites stay one line long.
   * The menus pick up the host's flyout theme resources automatically.
   *
   */
   class IMenuFactory : public anka::Core::Object::IObject
   {
      public:

         virtual ~IMenuFactory() noexcept override = default;

         virtual winrt::Microsoft::UI::Xaml::Controls::MenuFlyout
            createFlyout(std::span<const MenuCommand> commands) const = 0;

         // Shows the built menu on the element's context gesture (right
         // click, pen barrel, Shift+F10 / menu key).
         virtual void attachContextMenu(const winrt::Microsoft::UI::Xaml::UIElement& target,
                                        std::span<const MenuCommand> commands) const = 0;
   };
}

#endif // ANKA_GUI_MENU_IMENUFACTORY_HPP
