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

#ifndef ANKA_GUI_VIEWPORT_EXTENT_HPP
#define ANKA_GUI_VIEWPORT_EXTENT_HPP

namespace anka::GUI::Viewport
{
   // A width × height pair in one coordinate space; the caller decides whether
   // that space is device pixels or DIPs, and must compare like with like.
   // isEmpty() marks the "nothing to measure yet" states — an image not opened
   // yet, or a viewport before its first layout pass.
   struct Extent
   {
      double width  {0.0};
      double height {0.0};

      bool isEmpty(void) const
      {
         return width <= 0.0 || height <= 0.0;
      }
   };
}

#endif // ANKA_GUI_VIEWPORT_EXTENT_HPP
