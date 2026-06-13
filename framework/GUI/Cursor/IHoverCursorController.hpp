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

#ifndef ANKA_GUI_CURSOR_IHOVERCURSORCONTROLLER_HPP
#define ANKA_GUI_CURSOR_IHOVERCURSORCONTROLLER_HPP

#include "framework/Core/Object/IObject.hpp"

namespace anka::GUI::Cursor
{
   /*
   * Abstract Hover Cursor Controller Interface
   *
   * Watches the pointer over a host element and keeps the right cursor showing
   * by running an ordered set of cursor rules against whatever is under it.
   * The wiring starts when the controller is constructed and ends when it is
   * destroyed; this seam adds only the one thing a caller must drive by hand.
   *
   */
   class IHoverCursorController : public anka::Core::Object::IObject
   {
      public:

         virtual ~IHoverCursorController() noexcept override = default;

         // Re-evaluate the cursor for the element last under the pointer. Used
         // when the world changed beneath a still pointer — a zoom that just
         // made the image draggable — so the cursor updates without a move.
         virtual void refresh(void) = 0;
   };
}

#endif // ANKA_GUI_CURSOR_IHOVERCURSORCONTROLLER_HPP
