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
#include "OColorMatrixEffect.hpp"

#include "PixelCodec.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstddef>
#include <cstdint>

using namespace anka::Graphics::Effect;

using anka::Core::Object::ObjectIdentity;
using PixelCodec::Rgba;

// -----------------------------------------------------------------------------
// OColorMatrixEffect::Impl Definition
// -----------------------------------------------------------------------------
struct OColorMatrixEffect::Impl
{
   ObjectIdentity m_identity {L"ColorMatrixEffectObject"};

   const ColorMatrix m_matrix;

   explicit Impl(const ColorMatrix& matrix) :
      m_matrix {matrix}
   {}

   // One output channel: the matrix row's weighted sum of (R, G, B, A, 1).
   float channel(const Rgba& colour, std::size_t row) const
   {
      const auto& m {m_matrix.m};
      const std::size_t i {row * ColorMatrix::k_columns};

      return m[i] * colour.r + m[i + 1] * colour.g + m[i + 2] * colour.b
           + m[i + 3] * colour.a + m[i + 4];
   }

   Rgba map(const Rgba& colour) const
   {
      return {channel(colour, 0), channel(colour, 1), channel(colour, 2), channel(colour, 3)};
   }

   void transformPixel(std::uint8_t* pixel) const
   {
      PixelCodec::premultiply(map(PixelCodec::unpremultiply(pixel)), pixel);
   }

   void apply(const PixelView& frame) const
   {
      PixelCodec::forEachPixel(frame, [this](std::uint8_t* pixel) { transformPixel(pixel); });
   }
};

// -----------------------------------------------------------------------------
// OColorMatrixEffect Implementation
// -----------------------------------------------------------------------------
OColorMatrixEffect::OColorMatrixEffect(const ColorMatrix& matrix) :
   m_pImpl {std::make_unique<Impl>(matrix)}
{}
// -----------------------------------------------------------------------------
OColorMatrixEffect::~OColorMatrixEffect() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OColorMatrixEffect::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OColorMatrixEffect::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OColorMatrixEffect::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OColorMatrixEffect::apply(const PixelView& frame) const
{
   m_pImpl->apply(frame);
}
// -----------------------------------------------------------------------------
