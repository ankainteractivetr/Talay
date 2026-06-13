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

#ifndef TALAY_EFFECTS_OEFFECTCATALOG_HPP
#define TALAY_EFFECTS_OEFFECTCATALOG_HPP

#include "IEffectCatalog.hpp"

#include <memory>

namespace talay::Effects
{
   /*
   *
   * Concrete Effect Catalog Class
   *
   * Builds one shared, immutable effect per EffectKind up front (Flyweight:
   * effects are stateless, so every caller gets the same instance) and hands
   * them out by kind. Which engine realises a kind — colour matrix, lookup
   * table, convolution, vignette — is decided entirely here: adding an
   * effect is one preset and one registration line, and nothing above this
   * seam changes.
   *
   */
   class OEffectCatalog final : public IEffectCatalog
   {
      public:

         OEffectCatalog();

         OEffectCatalog(const OEffectCatalog& catalog) = delete;
         OEffectCatalog(OEffectCatalog&& catalog) noexcept = delete;

         ~OEffectCatalog() noexcept override;

         OEffectCatalog& operator=(const OEffectCatalog& catalog) = delete;
         OEffectCatalog& operator=(OEffectCatalog&& catalog) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IEffectCatalog overrides
         //

         std::shared_ptr<const anka::Graphics::Effect::IPixelEffect> effectFor(EffectKind kind) const override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_EFFECTS_OEFFECTCATALOG_HPP
