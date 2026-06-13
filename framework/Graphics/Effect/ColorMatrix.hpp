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

#ifndef ANKA_GRAPHICS_EFFECT_COLORMATRIX_HPP
#define ANKA_GRAPHICS_EFFECT_COLORMATRIX_HPP

#include <array>
#include <cstddef>

namespace anka::Graphics::Effect
{
   // A 4x5 colour transform over normalised, non-premultiplied RGBA: each
   // output channel is one row's weighted sum of (R, G, B, A, 1), the last
   // column being the constant offset. This is the shape every classic view
   // effect (grayscale, sepia, invert, brightness, ...) collapses into, so one
   // engine can serve them all and a new look is new data, not new code.
   struct ColorMatrix
   {
      static constexpr std::size_t k_rows    {4};
      static constexpr std::size_t k_columns {5};

      std::array<float, k_rows * k_columns> m {};
   };
}

#endif // ANKA_GRAPHICS_EFFECT_COLORMATRIX_HPP
