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

#ifndef ANKA_GRAPHICS_EFFECT_KERNELPRESETS_HPP
#define ANKA_GRAPHICS_EFFECT_KERNELPRESETS_HPP

#include "ConvolutionKernel.hpp"

namespace anka::Graphics::Effect::KernelPresets
{
   // The classic Laplacian sharpen: the centre amplified by what it differs
   // from its cross neighbours — edges pop, flat areas stay put.
   ConvolutionKernel sharpen(void);

   // A 3x3 Gaussian blur (1-2-1 binomial weights): the gentle soften that
   // smooths noise without smearing the image into mush.
   ConvolutionKernel soften(void);
}

#endif // ANKA_GRAPHICS_EFFECT_KERNELPRESETS_HPP
