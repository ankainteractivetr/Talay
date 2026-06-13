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

#include "pch.h"
#include "ColorMatrixPresets.hpp"

using anka::Graphics::Effect::ColorMatrix;

// -----------------------------------------------------------------------------
// Matrix builders — the parameterised shapes the named presets are points on.
// -----------------------------------------------------------------------------
namespace
{
   constexpr float k_lumaR {0.299f};
   constexpr float k_lumaG {0.587f};
   constexpr float k_lumaB {0.114f};

   // The standard saturation matrix: blends every channel between the image's
   // own luma (s = 0, grayscale) and itself (s = 1, identity); s > 1 overshoots
   // into vivid.
   ColorMatrix saturation(float s)
   {
      const float r {k_lumaR * (1.0f - s)};
      const float g {k_lumaG * (1.0f - s)};
      const float b {k_lumaB * (1.0f - s)};

      return {.m = {
         r + s, g,     b,     0.0f, 0.0f,
         r,     g + s, b,     0.0f, 0.0f,
         r,     g,     b + s, 0.0f, 0.0f,
         0.0f,  0.0f,  0.0f,  1.0f, 0.0f}};
   }

   // Independent gains per primary — enough for white-balance style shifts.
   ColorMatrix channelGains(float red, float green, float blue)
   {
      return {.m = {
         red,  0.0f,  0.0f, 0.0f, 0.0f,
         0.0f, green, 0.0f, 0.0f, 0.0f,
         0.0f, 0.0f,  blue, 0.0f, 0.0f,
         0.0f, 0.0f,  0.0f, 1.0f, 0.0f}};
   }
}

// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::grayscale(void)
{
   return saturation(0.0f);
}
// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::sepia(void)
{
   return {.m = {
      0.393f, 0.769f, 0.189f, 0.0f, 0.0f,
      0.349f, 0.686f, 0.168f, 0.0f, 0.0f,
      0.272f, 0.534f, 0.131f, 0.0f, 0.0f,
      0.0f,   0.0f,   0.0f,   1.0f, 0.0f}};
}
// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::invert(void)
{
   return {.m = {
      -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
       0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
       0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
       0.0f,  0.0f,  0.0f, 1.0f, 0.0f}};
}
// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::warm(void)
{
   return channelGains(1.10f, 1.02f, 0.86f);
}
// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::cool(void)
{
   return channelGains(0.88f, 1.00f, 1.12f);
}
// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::vibrant(void)
{
   return saturation(1.45f);
}
// -----------------------------------------------------------------------------
ColorMatrix anka::Graphics::Effect::ColorMatrixPresets::muted(void)
{
   return saturation(0.55f);
}
// -----------------------------------------------------------------------------
