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

#ifndef ANKA_GRAPHICS_EFFECT_EFFECTAPPLICATION_HPP
#define ANKA_GRAPHICS_EFFECT_EFFECTAPPLICATION_HPP

#include "framework/Graphics/Image/DecodedImage.hpp"
#include "IPixelEffect.hpp"

namespace anka::Graphics::Effect::EffectApplication
{
   // A copy of 'image' with 'effect' baked into its pixels; the source stays
   // untouched, which is what keeps view effects read-only end to end. An
   // empty image passes through empty, so the caller's failure path is one.
   anka::Graphics::Image::DecodedImage applied(const anka::Graphics::Image::DecodedImage& image,
                                               const IPixelEffect& effect);
}

#endif // ANKA_GRAPHICS_EFFECT_EFFECTAPPLICATION_HPP
