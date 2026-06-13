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

#ifndef ANKA_GRAPHICS_EFFECT_COLORMATRIXPRESETS_HPP
#define ANKA_GRAPHICS_EFFECT_COLORMATRIXPRESETS_HPP

#include "ColorMatrix.hpp"

namespace anka::Graphics::Effect::ColorMatrixPresets
{
   // The well-known looks as data (Flyweight source): each function returns
   // the matrix that, fed to the one colour-matrix engine, produces the named
   // effect. All of them leave alpha untouched.

   // Rec. 601 luma weights into all three channels — the classic photographic
   // grayscale, matched to how the eye weighs the primaries.
   ColorMatrix grayscale(void);

   // The classic sepia weights: the warm browns of an aged photograph.
   ColorMatrix sepia(void);

   // Photographic negative: every channel mirrored around full intensity.
   ColorMatrix invert(void);

   // Channel gains shifted toward red (warm) or blue (cool) — the quick
   // white-balance looks of golden-hour and overcast light.
   ColorMatrix warm(void);
   ColorMatrix cool(void);

   // Saturation moved off neutral: above one toward vivid, below one toward
   // pastel — both blends between the image and its own grayscale.
   ColorMatrix vibrant(void);
   ColorMatrix muted(void);
}

#endif // ANKA_GRAPHICS_EFFECT_COLORMATRIXPRESETS_HPP
