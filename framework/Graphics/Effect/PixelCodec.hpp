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

#ifndef ANKA_GRAPHICS_EFFECT_PIXELCODEC_HPP
#define ANKA_GRAPHICS_EFFECT_PIXELCODEC_HPP

#include "PixelView.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

namespace anka::Graphics::Effect::PixelCodec
{
   // The translation every per-pixel effect shares: between the stored bytes
   // (premultiplied BGRA) and the space colour maths is defined over
   // (normalised, non-premultiplied RGBA) — plus the frame iteration shells,
   // so each effect contributes only its own per-pixel transform.

   inline constexpr float       k_byteScale     {1.0f / 255.0f};
   inline constexpr std::size_t k_bytesPerPixel {4};

   // One pixel in normalised, non-premultiplied RGBA.
   struct Rgba
   {
      float r;
      float g;
      float b;
      float a;
   };

   inline float toUnit(std::uint8_t value)
   {
      return value * k_byteScale;
   }

   inline std::uint8_t toByte(float unit)
   {
      return static_cast<std::uint8_t>(std::clamp(unit, 0.0f, 1.0f) * 255.0f + 0.5f);
   }

   // Premultiplied BGRA bytes -> straight RGBA. A fully transparent pixel has
   // no colour left to recover; it stays black and the effect's offsets decide.
   inline Rgba unpremultiply(const std::uint8_t* pixel)
   {
      const float a {toUnit(pixel[3])};
      if (a == 0.0f)
         return {0.0f, 0.0f, 0.0f, 0.0f};

      return {toUnit(pixel[2]) / a, toUnit(pixel[1]) / a, toUnit(pixel[0]) / a, a};
   }

   // Straight RGBA -> premultiplied BGRA bytes. Channels saturate before the
   // alpha weighting, so an overdriven colour clips like film, not like math.
   inline void premultiply(const Rgba& colour, std::uint8_t* pixel)
   {
      const float a {std::clamp(colour.a, 0.0f, 1.0f)};

      pixel[0] = toByte(std::clamp(colour.b, 0.0f, 1.0f) * a);
      pixel[1] = toByte(std::clamp(colour.g, 0.0f, 1.0f) * a);
      pixel[2] = toByte(std::clamp(colour.r, 0.0f, 1.0f) * a);
      pixel[3] = toByte(a);
   }

   // Run 'operation(pixel)' over every pixel; rows are addressed by stride,
   // not width, so padded buffers stay intact.
   template <typename PixelOperation>
   void forEachPixel(const PixelView& frame, PixelOperation operation)
   {
      for (std::uint32_t y {0}; y < frame.height; ++y) {
         std::uint8_t* row {frame.pixels + static_cast<std::size_t>(y) * frame.stride};

         for (std::uint32_t x {0}; x < frame.width; ++x)
            operation(row + static_cast<std::size_t>(x) * k_bytesPerPixel);
      }
   }

   // As forEachPixel, for effects whose transform depends on the position
   // (vignette falloff, convolution neighbourhoods): 'operation(pixel, x, y)'.
   template <typename PixelOperation>
   void forEachPixelAt(const PixelView& frame, PixelOperation operation)
   {
      for (std::uint32_t y {0}; y < frame.height; ++y) {
         std::uint8_t* row {frame.pixels + static_cast<std::size_t>(y) * frame.stride};

         for (std::uint32_t x {0}; x < frame.width; ++x)
            operation(row + static_cast<std::size_t>(x) * k_bytesPerPixel, x, y);
      }
   }
}

#endif // ANKA_GRAPHICS_EFFECT_PIXELCODEC_HPP
