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
#include "OImagePresenter.hpp"

#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Surface.hpp"
#include "framework/Graphics/Renderer/ORenderer.hpp"
#include "framework/Graphics/Resource/OGraphicsResourceKeeper.hpp"
#include "framework/Graphics/Bitmap/OSurfaceBitmapFactory.hpp"
#include "framework/Graphics/Bitmap/ISurfaceBitmap.hpp"
#include "framework/Graphics/Image/ImageSequence.hpp"
#include "framework/Graphics/Image/DecodedImage.hpp"
#include "framework/Graphics/Effect/IPixelEffect.hpp"

#include <robuffer.h>
#include <winrt/Windows.Storage.Streams.h>
#include <d2d1.h>

#include <cstdint>
#include <limits>

using namespace anka::GUI::Presentation;

using anka::Core::Object::ObjectIdentity;
using anka::Graphics::SurfaceId;
using anka::Graphics::k_invalidSurface;
using anka::Graphics::Renderer::ORenderer;
using anka::Graphics::Resource::OGraphicsResourceKeeper;
using anka::Graphics::Bitmap::ISurfaceBitmap;
using anka::Graphics::Bitmap::OSurfaceBitmapFactory;
using anka::Graphics::Image::ImageSequence;
using anka::Graphics::Effect::IPixelEffect;

using winrt::Windows::Foundation::IInspectable;
using winrt::Microsoft::UI::Xaml::DispatcherTimer;
using winrt::Microsoft::UI::Xaml::Controls::Image;
using winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap;

namespace
{
   // XAML's "Auto": clears an explicit Width/Height back to size-to-content.
   constexpr double k_autoSize {std::numeric_limits<double>::quiet_NaN()};
}

// -----------------------------------------------------------------------------
// OImagePresenter::Impl Definition
// -----------------------------------------------------------------------------
struct OImagePresenter::Impl
{
   ObjectIdentity m_identity {L"ImagePresenterObject"};

   Image           m_target  {nullptr};
   SurfaceId       m_surface {k_invalidSurface};
   std::unique_ptr<ISurfaceBitmap> m_image;
   WriteableBitmap m_canvas  {nullptr};
   DispatcherTimer m_timer   {nullptr};

   // The read-only view effect baked into every canvas copy; null shows the
   // original pixels. Dropped by clear(), so each presented image opens clean.
   std::shared_ptr<const IPixelEffect> m_effect;

   // The timer's Tick delegate holds a raw pointer back to this Impl, so it must
   // never outlive it; stop it here in case teardown skips clear().
   ~Impl(void)
   {
      if (m_timer)
         m_timer.Stop();
   }

   // Replace whatever is shown with 'sequence': build its surface + frames, draw
   // the first frame, hand it to the Image, and start playback if it animates.
   // Clearing first, unconditionally, keeps the contract honest: a failed
   // present leaves nothing shown — never a stale predecessor.
   bool present(const ImageSequence& sequence)
   {
      clear();
      if (sequence.empty())
         return false;

      if (!build(sequence))
         return false;

      show();
      return true;
   }

   // Hand the built image to the XAML element: a fresh canvas with the first
   // frame on it, native sizing, and playback if it animates.
   void show(void)
   {
      prepareCanvas();
      blitCurrentFrame();
      m_target.Source(m_canvas);
      sizeTargetToCanvas();
      startPlayback();
   }

   // Pin the Image element's layout size to the canvas's pixel extent: the
   // content always exactly fills the element (an oversized image would
   // otherwise be clipped to the layout slot), so the host's viewport
   // transform alone decides the on-screen scale.
   void sizeTargetToCanvas(void)
   {
      m_target.Width(static_cast<double>(m_canvas.PixelWidth()));
      m_target.Height(static_cast<double>(m_canvas.PixelHeight()));
   }

   void resetTargetSize(void)
   {
      if (m_target) {
         m_target.Width(k_autoSize);
         m_target.Height(k_autoSize);
      }
   }

   // Create a surface sized to the image and upload its frames into it; the
   // surface must outlive m_image, so present() clears the previous one first.
   bool build(const ImageSequence& sequence)
   {
      const auto& first = sequence.frame(0).image();
      const D2D1_SIZE_U size = D2D1::SizeU(first.width(), first.height());

      m_surface = OGraphicsResourceKeeper::getInstance().createSurface(size);
      m_image   = OSurfaceBitmapFactory::getInstance().create(m_surface, sequence);
      return m_image != nullptr;
   }

   // A WriteableBitmap the exact pixel size of the image; each frame is copied
   // into it and the Image shows it at the element's (native) layout size.
   void prepareCanvas(void)
   {
      const D2D1_SIZE_U size = m_image->pixelSize();
      m_canvas = WriteableBitmap(static_cast<std::int32_t>(size.width),
                                 static_cast<std::int32_t>(size.height));
   }

   // Draw the current frame 1:1 into the surface, then copy it into the canvas.
   void blitCurrentFrame(void)
   {
      ORenderer& renderer = ORenderer::getInstance();
      const D2D1_SIZE_U size = m_image->pixelSize();
      const D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, static_cast<float>(size.width),
                                                       static_cast<float>(size.height));
      renderer.beginDraw(m_surface);
      renderer.clear(D2D1::ColorF(0.0f, 0.0f, 0.0f, 0.0f));
      renderer.drawBitmap(rect, rect, m_image->currentFrame());
      renderer.endDraw();
      copyToCanvas(size);
   }

   // Hand the rendered pixels to the WriteableBitmap's backing buffer and flush.
   void copyToCanvas(const D2D1_SIZE_U& size)
   {
      const auto buffer = m_canvas.PixelBuffer();
      auto access = buffer.as<::Windows::Storage::Streams::IBufferByteAccess>();
      std::uint8_t* bytes {nullptr};
      winrt::check_hresult(access->Buffer(&bytes));
      ORenderer::getInstance().copyPixels(m_surface, bytes, size.width * 4, buffer.Capacity());
      applyEffect(bytes, size);
      m_canvas.Invalidate();
   }

   // The view effect touches only the canvas copy — the surface and the
   // uploaded frames keep the original pixels, so dropping it restores them.
   void applyEffect(std::uint8_t* bytes, const D2D1_SIZE_U& size) const
   {
      if (!m_effect)
         return;

      m_effect->apply({.pixels = bytes, .width = size.width,
                       .height = size.height, .stride = size.width * 4});
   }

   // Swap the active effect and refresh the frame on screen, so the change
   // (or its removal) is visible immediately.
   void setEffect(std::shared_ptr<const IPixelEffect> effect)
   {
      m_effect = std::move(effect);
      if (m_image)
         blitCurrentFrame();
   }

   // Animations get a repeating timer keyed to the current frame's delay; a still
   // draws once and needs none.
   void startPlayback(void)
   {
      if (!m_image->isAnimated())
         return;

      m_timer = DispatcherTimer();
      m_timer.Tick({this, &Impl::onFrameTick});
      m_timer.Interval(m_image->currentDelay());
      m_timer.Start();
   }

   // Advance the animation; re-blit on a frame change and re-arm for the next
   // delay, or leave the timer stopped once a finite loop is exhausted.
   void onFrameTick(const IInspectable&, const IInspectable&)
   {
      m_timer.Stop();
      if (!m_image || !m_image->advance())
         return;

      blitCurrentFrame();
      m_timer.Interval(m_image->currentDelay());
      m_timer.Start();
   }

   // Stop any playback and release the current image before its surface, so the
   // surface's render target is free of the bitmaps that live on it.
   void clear(void)
   {
      if (m_timer) {
         m_timer.Stop();
         m_timer = nullptr;
      }

      m_image.reset();
      m_effect.reset();
      releaseCanvas();
      releaseSurface();
      resetTargetSize();
   }

   // Detach and drop the canvas so canvas() is honestly null while nothing is
   // shown — and a failed present leaves a blank target, not a stale frame.
   void releaseCanvas(void)
   {
      if (m_target)
         m_target.Source(nullptr);

      m_canvas = nullptr;
   }

   void releaseSurface(void)
   {
      if (m_surface == k_invalidSurface)
         return;

      OGraphicsResourceKeeper::getInstance().destroySurface(m_surface);
      m_surface = k_invalidSurface;
   }
};

// -----------------------------------------------------------------------------
// OImagePresenter Implementation
// -----------------------------------------------------------------------------
OImagePresenter::OImagePresenter() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OImagePresenter::~OImagePresenter() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OImagePresenter::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OImagePresenter::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImagePresenter::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OImagePresenter::attach(const Image& target)
{
   m_pImpl->m_target = target;
}
// -----------------------------------------------------------------------------
bool OImagePresenter::present(const ImageSequence& sequence)
{
   return m_pImpl->present(sequence);
}
// -----------------------------------------------------------------------------
void OImagePresenter::clear(void)
{
   m_pImpl->clear();
}
// -----------------------------------------------------------------------------
void OImagePresenter::setEffect(std::shared_ptr<const IPixelEffect> effect)
{
   m_pImpl->setEffect(std::move(effect));
}
// -----------------------------------------------------------------------------
WriteableBitmap OImagePresenter::canvas(void) const
{
   return m_pImpl->m_canvas;
}
// -----------------------------------------------------------------------------
