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

#ifndef TALAY_EFFECTS_EFFECTKIND_HPP
#define TALAY_EFFECTS_EFFECTKIND_HPP

namespace talay::Effects
{
   // Every view effect the Effects menu offers; None is the original image.
   // The menu picks a kind, the catalog turns it into the framework effect —
   // so this enum is the whole vocabulary the window needs. Grouped the way
   // the menu groups them: tone, colour, light, detail.
   enum class EffectKind
   {
      None,
      Grayscale,
      Sepia,
      Invert,
      Warm,
      Cool,
      Vibrant,
      Muted,
      Brighten,
      Darken,
      Sharpen,
      Soften,
      Vignette
   };
}

#endif // TALAY_EFFECTS_EFFECTKIND_HPP
