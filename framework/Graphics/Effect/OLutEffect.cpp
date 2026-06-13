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
#include "OLutEffect.hpp"

#include "PixelCodec.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>

using namespace anka::Graphics::Effect;

using anka::Core::Object::ObjectIdentity;
using PixelCodec::Rgba;

// -----------------------------------------------------------------------------
// OLutEffect::Impl Definition
// -----------------------------------------------------------------------------
struct OLutEffect::Impl
{
   ObjectIdentity m_identity {L"LutEffectObject"};

   const LookupTable m_table;

   explicit Impl(const LookupTable& table) :
      m_table {table}
   {}

   // One straight channel through the curve: unit -> byte index -> unit.
   float curved(float unit) const
   {
      const auto index {static_cast<std::size_t>(std::clamp(unit, 0.0f, 1.0f) * 255.0f + 0.5f)};

      return m_table.values[index] * PixelCodec::k_byteScale;
   }

   void transformPixel(std::uint8_t* pixel) const
   {
      Rgba colour {PixelCodec::unpremultiply(pixel)};

      colour.r = curved(colour.r);
      colour.g = curved(colour.g);
      colour.b = curved(colour.b);
      PixelCodec::premultiply(colour, pixel);
   }

   void apply(const PixelView& frame) const
   {
      PixelCodec::forEachPixel(frame, [this](std::uint8_t* pixel) { transformPixel(pixel); });
   }
};

// -----------------------------------------------------------------------------
// OLutEffect Implementation
// -----------------------------------------------------------------------------
OLutEffect::OLutEffect(const LookupTable& table) :
   m_pImpl {std::make_unique<Impl>(table)}
{}
// -----------------------------------------------------------------------------
OLutEffect::~OLutEffect() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OLutEffect::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OLutEffect::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OLutEffect::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OLutEffect::apply(const PixelView& frame) const
{
   m_pImpl->apply(frame);
}
// -----------------------------------------------------------------------------
