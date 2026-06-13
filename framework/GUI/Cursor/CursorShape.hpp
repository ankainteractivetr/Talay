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

#ifndef ANKA_GUI_CURSOR_CURSORSHAPE_HPP
#define ANKA_GUI_CURSOR_CURSORSHAPE_HPP

namespace anka::GUI::Cursor
{
   // The cursor a hover rule can ask for, kept abstract so the policy layer
   // never names a platform cursor. Default means "nothing special — show the
   // plain arrow"; the surface that owns the cursor maps each to a concrete
   // system cursor (Hand for clickable things, Move for a draggable image).
   enum class CursorShape
   {
      Default,
      Hand,
      Move
   };
}

#endif // ANKA_GUI_CURSOR_CURSORSHAPE_HPP
