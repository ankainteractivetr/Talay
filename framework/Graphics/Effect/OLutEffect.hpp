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

#ifndef ANKA_GRAPHICS_EFFECT_OLUTEFFECT_HPP
#define ANKA_GRAPHICS_EFFECT_OLUTEFFECT_HPP

#include "IPixelEffect.hpp"
#include "LookupTable.hpp"

#include <memory>

namespace anka::Graphics::Effect
{
   /*
   *
   * Concrete Lookup Table Effect Class
   *
   * The engine behind every tone-curve view effect: each pixel is
   * un-premultiplied, its R, G and B looked up through the injected table,
   * and re-premultiplied — alpha is never curved. Which curve it applies is
   * purely the table handed in (see LutPresets), so a new tone look is new
   * data, not a new class. Immutable after construction, hidden behind a
   * pimpl.
   *
   */
   class OLutEffect final : public IPixelEffect
   {
      public:

         explicit OLutEffect(const LookupTable& table);

         OLutEffect(const OLutEffect& effect) = delete;
         OLutEffect(OLutEffect&& effect) noexcept = delete;

         ~OLutEffect() noexcept override;

         OLutEffect& operator=(const OLutEffect& effect) = delete;
         OLutEffect& operator=(OLutEffect&& effect) noexcept = delete;

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

#endif // ANKA_GRAPHICS_EFFECT_OLUTEFFECT_HPP
