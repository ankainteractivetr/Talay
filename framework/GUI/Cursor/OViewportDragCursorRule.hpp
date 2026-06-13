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

#ifndef ANKA_GUI_CURSOR_OVIEWPORTDRAGCURSORRULE_HPP
#define ANKA_GUI_CURSOR_OVIEWPORTDRAGCURSORRULE_HPP

#include "ICursorRule.hpp"

#include <functional>

namespace anka::GUI::Cursor
{
   // Answers "can the surface be dragged right now?" — injected so this rule
   // stays ignorant of the viewport, asking only a yes/no it is handed.
   using DragPredicate = std::function<bool(void)>;

   /*
   *
   * Concrete Viewport-Drag Cursor Rule
   *
   * Asks for the Move cursor while the pointer is over a given surface AND the
   * injected predicate says it can be dragged — for an image viewport, that is
   * exactly when the image overflows its frame and a drag would pan it. When
   * the image fits, the predicate is false and the rule passes, so a plain
   * arrow shows. The surface and the predicate are injected (DIP): the rule
   * knows nothing about viewports, only about a draggable region.
   *
   */
   class OViewportDragCursorRule final : public ICursorRule
   {
      public:

         OViewportDragCursorRule(winrt::Microsoft::UI::Xaml::UIElement surface,
                                 DragPredicate isDraggable);

         std::optional<CursorShape> resolve(
            const winrt::Microsoft::UI::Xaml::DependencyObject& hovered) const override;

      private:

         winrt::Microsoft::UI::Xaml::DependencyObject m_surface;
         DragPredicate                                m_isDraggable;
   };
}

#endif // ANKA_GUI_CURSOR_OVIEWPORTDRAGCURSORRULE_HPP
