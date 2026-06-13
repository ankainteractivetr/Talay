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

#ifndef ANKA_GUI_CURSOR_OCLICKABLECURSORRULE_HPP
#define ANKA_GUI_CURSOR_OCLICKABLECURSORRULE_HPP

#include "ICursorRule.hpp"

namespace anka::GUI::Cursor
{
   /*
   *
   * Concrete Clickable Cursor Rule
   *
   * Asks for the Hand cursor whenever the pointer is over something the user
   * can click: any enabled ButtonBase (button, toggle, repeat, hyperlink) or
   * SelectorItem (a grid / list tile). It walks up from the hovered element so
   * a click that lands on a button's inner content still reads as the button,
   * and a disabled control is left alone. Stateless — one instance serves the
   * whole window.
   *
   */
   class OClickableCursorRule final : public ICursorRule
   {
      public:

         std::optional<CursorShape> resolve(
            const winrt::Microsoft::UI::Xaml::DependencyObject& hovered) const override;
   };
}

#endif // ANKA_GUI_CURSOR_OCLICKABLECURSORRULE_HPP
