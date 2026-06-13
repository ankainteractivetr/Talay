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

#ifndef TALAY_EFFECTS_IEFFECTCATALOG_HPP
#define TALAY_EFFECTS_IEFFECTCATALOG_HPP

#include "EffectKind.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <memory>

// Forward-declared so the window depends only on the effect abstraction; the
// concrete effects are pulled in by the catalog implementation.
namespace anka::Graphics::Effect { class IPixelEffect; }

namespace talay::Effects
{
   /*
   * Abstract Effect Catalog Interface
   *
   * Turns a menu choice (EffectKind) into the framework pixel effect that
   * realises it. The window never learns which effect class or colour matrix
   * sits behind a menu item, so a new effect extends the catalog and the
   * menu — never the window's logic.
   *
   */
   class IEffectCatalog : public anka::Core::Object::IObject
   {
      public:

         virtual ~IEffectCatalog() noexcept override = default;

         // The shared, immutable effect for 'kind'; null for EffectKind::None,
         // which means "show the original pixels".
         virtual std::shared_ptr<const anka::Graphics::Effect::IPixelEffect> effectFor(EffectKind kind) const = 0;
   };
}

#endif // TALAY_EFFECTS_IEFFECTCATALOG_HPP
