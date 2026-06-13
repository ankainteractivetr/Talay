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

#ifndef ANKA_GRAPHICS_EFFECT_OCOLORMATRIXEFFECT_HPP
#define ANKA_GRAPHICS_EFFECT_OCOLORMATRIXEFFECT_HPP

#include "IPixelEffect.hpp"
#include "ColorMatrix.hpp"

#include <memory>

namespace anka::Graphics::Effect
{
   /*
   *
   * Concrete Colour Matrix Effect Class
   *
   * The one engine behind every matrix-shaped view effect: each pixel is
   * un-premultiplied, mapped through the injected ColorMatrix in normalised
   * RGBA space, clamped and re-premultiplied — so the maths stays correct
   * wherever alpha is partial. Which look it produces is decided purely by
   * the matrix handed in (see ColorMatrixPresets); new looks are new data,
   * not new classes. Immutable after construction, hidden behind a pimpl.
   *
   */
   class OColorMatrixEffect final : public IPixelEffect
   {
      public:

         explicit OColorMatrixEffect(const ColorMatrix& matrix);

         OColorMatrixEffect(const OColorMatrixEffect& effect) = delete;
         OColorMatrixEffect(OColorMatrixEffect&& effect) noexcept = delete;

         ~OColorMatrixEffect() noexcept override;

         OColorMatrixEffect& operator=(const OColorMatrixEffect& effect) = delete;
         OColorMatrixEffect& operator=(OColorMatrixEffect&& effect) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IPixelEffect overrides
         //

         void apply(const PixelView& frame) const override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GRAPHICS_EFFECT_OCOLORMATRIXEFFECT_HPP
