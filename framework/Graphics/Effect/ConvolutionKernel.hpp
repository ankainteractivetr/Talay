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

#ifndef ANKA_GRAPHICS_EFFECT_CONVOLUTIONKERNEL_HPP
#define ANKA_GRAPHICS_EFFECT_CONVOLUTIONKERNEL_HPP

#include <array>
#include <cstddef>

namespace anka::Graphics::Effect
{
   // A 3x3 neighbourhood filter as data, row-major, weights expected to sum
   // to one so overall brightness is preserved. The shape every classic
   // detail effect (sharpen, blur, emboss, edge detect) collapses into, so
   // one engine serves them all and a new look is a new set of weights.
   struct ConvolutionKernel
   {
      static constexpr std::size_t k_size {3};
      static constexpr std::size_t k_taps {k_size * k_size};

      std::array<float, k_taps> weights {};
   };
}

#endif // ANKA_GRAPHICS_EFFECT_CONVOLUTIONKERNEL_HPP
