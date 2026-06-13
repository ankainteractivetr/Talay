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

#ifndef ANKA_GUI_CURSOR_ICURSORRULE_HPP
#define ANKA_GUI_CURSOR_ICURSORRULE_HPP

#include <winrt/Microsoft.UI.Xaml.h>

#include <optional>

#include "CursorShape.hpp"

namespace anka::GUI::Cursor
{
   /*
   * Abstract Cursor Rule Interface (Strategy + Chain of Responsibility)
   *
   * One reason the cursor might change. Given the element under the pointer,
   * a rule either claims it with a shape or passes (nullopt) so the next rule
   * may try. The controller consults rules in priority order and takes the
   * first claim. Kept a stateless, identity-less strategy (ISP) so new reasons
   * — a resize grip, a link, a disabled overlay — can be added without
   * touching the controller (OCP).
   *
   */
   class ICursorRule
   {
      public:

         virtual ~ICursorRule() noexcept = default;

         // The cursor this rule wants for 'hovered' (the deepest element under
         // the pointer), or nullopt to let the next rule decide.
         virtual std::optional<CursorShape> resolve(
            const winrt::Microsoft::UI::Xaml::DependencyObject& hovered) const = 0;
   };
}

#endif // ANKA_GUI_CURSOR_ICURSORRULE_HPP
