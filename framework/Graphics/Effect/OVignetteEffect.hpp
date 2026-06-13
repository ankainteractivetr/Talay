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

#ifndef ANKA_GRAPHICS_EFFECT_OVIGNETTEEFFECT_HPP
#define ANKA_GRAPHICS_EFFECT_OVIGNETTEEFFECT_HPP

#include "IPixelEffect.hpp"

#include <memory>

namespace anka::Graphics::Effect
{
   /*
   *
   * Concrete Vignette Effect Class
   *
   * The classic photographic edge darkening: full brightness at the centre
   * falling off quadratically toward the corners. 'strength' (0..1) is how
   * dark the far corners get — 0 leaves the image untouched, 1 fades the
   * corners to black. Scaling premultiplied colour by a factor <= 1 keeps
   * the premultiplied invariant by construction, and alpha is never touched.
   * Immutable after construction, hidden behind a pimpl.
   *
   */
   class OVignetteEffect final : public IPixelEffect
   {
      public:

         explicit OVignetteEffect(float strength);

         OVignetteEffect(const OVignetteEffect& effect) = delete;
         OVignetteEffect(OVignetteEffect&& effect) noexcept = delete;

         ~OVignetteEffect() noexcept override;

         OVignetteEffect& operator=(const OVignetteEffect& effect) = delete;
         OVignetteEffect& operator=(OVignetteEffect&& effect) noexcept = delete;

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

#endif // ANKA_GRAPHICS_EFFECT_OVIGNETTEEFFECT_HPP
