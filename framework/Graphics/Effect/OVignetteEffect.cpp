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
#include "OVignetteEffect.hpp"

#include "PixelCodec.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cstdint>

using namespace anka::Graphics::Effect;

using anka::Core::Object::ObjectIdentity;

// -----------------------------------------------------------------------------
// OVignetteEffect::Impl Definition
// -----------------------------------------------------------------------------
struct OVignetteEffect::Impl
{
   ObjectIdentity m_identity {L"VignetteEffectObject"};

   const float m_strength;

   explicit Impl(float strength) :
      m_strength {std::clamp(strength, 0.0f, 1.0f)}
   {}

   // The frame's falloff field: centre point plus the inverse of the squared
   // centre-to-corner distance, so the corners land exactly at full strength.
   struct Geometry
   {
      float centreX;
      float centreY;
      float inverseReach;
   };

   static Geometry makeGeometry(const PixelView& frame)
   {
      const float centreX {frame.width * 0.5f};
      const float centreY {frame.height * 0.5f};

      return {centreX, centreY, 1.0f / (centreX * centreX + centreY * centreY)};
   }

   // 1 at the centre falling quadratically to (1 - strength) at the corners.
   float falloff(const Geometry& geometry, std::uint32_t x, std::uint32_t y) const
   {
      const float dx {(x + 0.5f) - geometry.centreX};
      const float dy {(y + 0.5f) - geometry.centreY};

      return 1.0f - m_strength * (dx * dx + dy * dy) * geometry.inverseReach;
   }

   // Scale the premultiplied colour only — alpha stays, so transparency is
   // untouched and the premultiplied invariant holds by construction.
   static void darken(std::uint8_t* pixel, float factor)
   {
      pixel[0] = static_cast<std::uint8_t>(pixel[0] * factor + 0.5f);
      pixel[1] = static_cast<std::uint8_t>(pixel[1] * factor + 0.5f);
      pixel[2] = static_cast<std::uint8_t>(pixel[2] * factor + 0.5f);
   }

   void apply(const PixelView& frame) const
   {
      if (frame.width == 0 || frame.height == 0)
         return;

      const Geometry geometry {makeGeometry(frame)};
      PixelCodec::forEachPixelAt(frame, [&, this](std::uint8_t* pixel, std::uint32_t x, std::uint32_t y)
         { darken(pixel, falloff(geometry, x, y)); });
   }
};

// -----------------------------------------------------------------------------
// OVignetteEffect Implementation
// -----------------------------------------------------------------------------
OVignetteEffect::OVignetteEffect(float strength) :
   m_pImpl {std::make_unique<Impl>(strength)}
{}
// -----------------------------------------------------------------------------
OVignetteEffect::~OVignetteEffect() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OVignetteEffect::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OVignetteEffect::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OVignetteEffect::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OVignetteEffect::apply(const PixelView& frame) const
{
   m_pImpl->apply(frame);
}
// -----------------------------------------------------------------------------
