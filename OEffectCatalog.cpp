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
#include "OEffectCatalog.hpp"

#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Effect/ColorMatrixPresets.hpp"
#include "framework/Graphics/Effect/KernelPresets.hpp"
#include "framework/Graphics/Effect/LutPresets.hpp"
#include "framework/Graphics/Effect/OColorMatrixEffect.hpp"
#include "framework/Graphics/Effect/OConvolutionEffect.hpp"
#include "framework/Graphics/Effect/OLutEffect.hpp"
#include "framework/Graphics/Effect/OVignetteEffect.hpp"

#include <unordered_map>
#include <utility>

using namespace talay::Effects;

using anka::Core::Object::ObjectIdentity;
using anka::Graphics::Effect::IPixelEffect;
using anka::Graphics::Effect::OColorMatrixEffect;
using anka::Graphics::Effect::OConvolutionEffect;
using anka::Graphics::Effect::OLutEffect;
using anka::Graphics::Effect::OVignetteEffect;

namespace MatrixPresets = anka::Graphics::Effect::ColorMatrixPresets;
namespace KernelPresets = anka::Graphics::Effect::KernelPresets;
namespace LutPresets    = anka::Graphics::Effect::LutPresets;

namespace
{
   // The tuning points of the parameterised effects, chosen to read as one
   // obvious step away from the original.
   constexpr float k_brightenGamma    {0.72f};
   constexpr float k_darkenGamma      {1.40f};
   constexpr float k_vignetteStrength {0.55f};
}

// -----------------------------------------------------------------------------
// OEffectCatalog::Impl Definition
// -----------------------------------------------------------------------------
struct OEffectCatalog::Impl
{
   ObjectIdentity m_identity {L"EffectCatalogObject"};

   // One shared instance per kind (Flyweight) — effects are immutable, so
   // these serve every presenter and share alike for the catalog's lifetime.
   std::unordered_map<EffectKind, std::shared_ptr<const IPixelEffect>> m_effects;

   Impl(void)
   {
      addToneEffects();
      addColourEffects();
      addLightEffects();
      addDetailEffects();
   }

   void add(EffectKind kind, std::shared_ptr<const IPixelEffect> effect)
   {
      m_effects.emplace(kind, std::move(effect));
   }

   void addToneEffects(void)
   {
      add(EffectKind::Grayscale, std::make_shared<const OColorMatrixEffect>(MatrixPresets::grayscale()));
      add(EffectKind::Sepia,     std::make_shared<const OColorMatrixEffect>(MatrixPresets::sepia()));
      add(EffectKind::Invert,    std::make_shared<const OColorMatrixEffect>(MatrixPresets::invert()));
   }

   void addColourEffects(void)
   {
      add(EffectKind::Warm,    std::make_shared<const OColorMatrixEffect>(MatrixPresets::warm()));
      add(EffectKind::Cool,    std::make_shared<const OColorMatrixEffect>(MatrixPresets::cool()));
      add(EffectKind::Vibrant, std::make_shared<const OColorMatrixEffect>(MatrixPresets::vibrant()));
      add(EffectKind::Muted,   std::make_shared<const OColorMatrixEffect>(MatrixPresets::muted()));
   }

   void addLightEffects(void)
   {
      add(EffectKind::Brighten, std::make_shared<const OLutEffect>(LutPresets::gamma(k_brightenGamma)));
      add(EffectKind::Darken,   std::make_shared<const OLutEffect>(LutPresets::gamma(k_darkenGamma)));
   }

   void addDetailEffects(void)
   {
      add(EffectKind::Sharpen,  std::make_shared<const OConvolutionEffect>(KernelPresets::sharpen()));
      add(EffectKind::Soften,   std::make_shared<const OConvolutionEffect>(KernelPresets::soften()));
      add(EffectKind::Vignette, std::make_shared<const OVignetteEffect>(k_vignetteStrength));
   }

   std::shared_ptr<const IPixelEffect> effectFor(EffectKind kind) const
   {
      const auto found {m_effects.find(kind)};

      return (found != m_effects.end()) ? found->second : nullptr;   // None: the original pixels
   }
};

// -----------------------------------------------------------------------------
// OEffectCatalog Implementation
// -----------------------------------------------------------------------------
OEffectCatalog::OEffectCatalog() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OEffectCatalog::~OEffectCatalog() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OEffectCatalog::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OEffectCatalog::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OEffectCatalog::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
std::shared_ptr<const IPixelEffect> OEffectCatalog::effectFor(EffectKind kind) const
{
   return m_pImpl->effectFor(kind);
}
// -----------------------------------------------------------------------------
