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

#ifndef ANKA_GRAPHICS_EFFECT_PIXELVIEW_HPP
#define ANKA_GRAPHICS_EFFECT_PIXELVIEW_HPP

#include <cstdint>

namespace anka::Graphics::Effect
{
   // A non-owning, mutable window onto one frame of 32-bit premultiplied BGRA
   // pixels (top-down), described exactly like DecodedImage: width and height
   // in pixels, stride in bytes per row. Effects transform these bytes in
   // place; whoever owns the buffer decides what "in place" means — handing in
   // a throwaway copy is what keeps a source image read-only.
   struct PixelView
   {
      std::uint8_t* pixels {nullptr};
      std::uint32_t width  {0};
      std::uint32_t height {0};
      std::uint32_t stride {0};
   };
}

#endif // ANKA_GRAPHICS_EFFECT_PIXELVIEW_HPP
