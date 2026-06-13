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
#include "OConvolutionEffect.hpp"

#include "PixelCodec.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

using namespace anka::Graphics::Effect;

using anka::Core::Object::ObjectIdentity;

namespace
{
   constexpr std::size_t k_channels {4};

   // Tap index -> neighbourhood offset, row-major around the centre.
   std::int64_t columnOffset(std::size_t tap)
   {
      return static_cast<std::int64_t>(tap % ConvolutionKernel::k_size) - 1;
   }

   std::int64_t rowOffset(std::size_t tap)
   {
      return static_cast<std::int64_t>(tap / ConvolutionKernel::k_size) - 1;
   }
}

// -----------------------------------------------------------------------------
// OConvolutionEffect::Impl Definition
// -----------------------------------------------------------------------------
struct OConvolutionEffect::Impl
{
   ObjectIdentity m_identity {L"ConvolutionEffectObject"};

   const ConvolutionKernel m_kernel;

   explicit Impl(const ConvolutionKernel& kernel) :
      m_kernel {kernel}
   {}

   // Taps need untouched neighbours, so reads come from a snapshot of the
   // frame while the results land in the frame itself.
   void apply(const PixelView& frame) const
   {
      if (frame.width == 0 || frame.height == 0)
         return;

      const std::vector<std::uint8_t> snapshot {frame.pixels, frame.pixels + frameBytes(frame)};
      PixelCodec::forEachPixelAt(frame, [&, this](std::uint8_t* pixel, std::uint32_t x, std::uint32_t y)
         { convolvePixel(snapshot, frame, pixel, x, y); });
   }

   static std::size_t frameBytes(const PixelView& frame)
   {
      return static_cast<std::size_t>(frame.stride) * frame.height;
   }

   void convolvePixel(const std::vector<std::uint8_t>& snapshot, const PixelView& frame,
                      std::uint8_t* pixel, std::uint32_t x, std::uint32_t y) const
   {
      float sums[k_channels] {};

      for (std::size_t tap {0}; tap < ConvolutionKernel::k_taps; ++tap)
         accumulateTap(sums, snapshot, frame, x, y, tap);

      storeClamped(sums, pixel);
   }

   void accumulateTap(float (&sums)[k_channels], const std::vector<std::uint8_t>& snapshot,
                      const PixelView& frame, std::uint32_t x, std::uint32_t y, std::size_t tap) const
   {
      const std::uint8_t* neighbour {neighbourAt(snapshot, frame, x, y, tap)};
      const float weight {m_kernel.weights[tap]};

      for (std::size_t channel {0}; channel < k_channels; ++channel)
         sums[channel] += weight * neighbour[channel];
   }

   // Edge pixels replicate their nearest neighbour (clamp-to-edge), so the
   // border keeps the image's own colours instead of bleeding black in.
   static const std::uint8_t* neighbourAt(const std::vector<std::uint8_t>& snapshot, const PixelView& frame,
                                          std::uint32_t x, std::uint32_t y, std::size_t tap)
   {
      const auto sx {std::clamp<std::int64_t>(x + columnOffset(tap), 0, frame.width - 1)};
      const auto sy {std::clamp<std::int64_t>(y + rowOffset(tap), 0, frame.height - 1)};

      return snapshot.data() + static_cast<std::size_t>(sy) * frame.stride
                             + static_cast<std::size_t>(sx) * PixelCodec::k_bytesPerPixel;
   }

   // Clamp colour back under the (filtered) alpha: premultiplied BGRA may
   // never carry more colour than coverage, or halos would oversaturate at
   // soft edges.
   static void storeClamped(const float (&sums)[k_channels], std::uint8_t* pixel)
   {
      const float alpha {std::clamp(sums[3], 0.0f, 255.0f)};

      pixel[0] = static_cast<std::uint8_t>(std::clamp(sums[0], 0.0f, alpha) + 0.5f);
      pixel[1] = static_cast<std::uint8_t>(std::clamp(sums[1], 0.0f, alpha) + 0.5f);
      pixel[2] = static_cast<std::uint8_t>(std::clamp(sums[2], 0.0f, alpha) + 0.5f);
      pixel[3] = static_cast<std::uint8_t>(alpha + 0.5f);
   }
};

// -----------------------------------------------------------------------------
// OConvolutionEffect Implementation
// -----------------------------------------------------------------------------
OConvolutionEffect::OConvolutionEffect(const ConvolutionKernel& kernel) :
   m_pImpl {std::make_unique<Impl>(kernel)}
{}
// -----------------------------------------------------------------------------
OConvolutionEffect::~OConvolutionEffect() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OConvolutionEffect::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OConvolutionEffect::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OConvolutionEffect::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OConvolutionEffect::apply(const PixelView& frame) const
{
   m_pImpl->apply(frame);
}
// -----------------------------------------------------------------------------
