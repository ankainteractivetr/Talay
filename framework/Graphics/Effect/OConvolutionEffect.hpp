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

#ifndef ANKA_GRAPHICS_EFFECT_OCONVOLUTIONEFFECT_HPP
#define ANKA_GRAPHICS_EFFECT_OCONVOLUTIONEFFECT_HPP

#include "IPixelEffect.hpp"
#include "ConvolutionKernel.hpp"

#include <memory>

namespace anka::Graphics::Effect
{
   /*
   *
   * Concrete Convolution Effect Class
   *
   * The engine behind every neighbourhood view effect: each output pixel is
   * the kernel-weighted sum of its 3x3 surroundings, read from a snapshot of
   * the frame so writes never feed back into reads. It filters the
   * premultiplied channels directly — the mathematically correct space for
   * linear filtering — and clamps colour back under alpha so sharpening
   * halos cannot break the premultiplied invariant. Which look it produces
   * is purely the kernel handed in (see KernelPresets). Immutable after
   * construction, hidden behind a pimpl.
   *
   */
   class OConvolutionEffect final : public IPixelEffect
   {
      public:

         explicit OConvolutionEffect(const ConvolutionKernel& kernel);

         OConvolutionEffect(const OConvolutionEffect& effect) = delete;
         OConvolutionEffect(OConvolutionEffect&& effect) noexcept = delete;

         ~OConvolutionEffect() noexcept override;

         OConvolutionEffect& operator=(const OConvolutionEffect& effect) = delete;
         OConvolutionEffect& operator=(OConvolutionEffect&& effect) noexcept = delete;

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

#endif // ANKA_GRAPHICS_EFFECT_OCONVOLUTIONEFFECT_HPP
