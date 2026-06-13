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
#include "OImageViewport.hpp"

// PointerPoint's Properties()/Position() accessors are declared with a deferred
// 'auto' return in the .0.h forward header; this brings in their definitions.
#include <winrt/Microsoft.UI.Input.h>

#include "OFitOrNativeFraming.hpp"
#include "PanBounds.hpp"

#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cmath>

using namespace anka::Core::Object;
using namespace anka::GUI::Viewport;

using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::Foundation::Point;
using winrt::Microsoft::UI::Xaml::FrameworkElement;
using winrt::Microsoft::UI::Xaml::SizeChangedEventArgs;
using winrt::Microsoft::UI::Xaml::UIElement;
using winrt::Microsoft::UI::Xaml::XamlRoot;
using winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs;
using winrt::Microsoft::UI::Xaml::Media::CompositeTransform;

namespace
{
   constexpr double k_nativePercent {100.0};
   constexpr double k_quarterTurn   {90.0};
   constexpr double k_fullTurn      {360.0};
   constexpr double k_wheelZoomStep {1.15};    // one notch matches the toolbar +/- step
   constexpr double k_wheelNotch    {120.0};   // WHEEL_DELTA: one detent of a classic wheel

   // How far the image point under 'anchor' slides when the content scales by
   // 'growth' about its centre: the anchor's offset from the centre stretches
   // by the growth, and the negated stretch is the pan that cancels the slide.
   double anchorDrift(double anchor, double centre, double growth)
   {
      return (anchor - centre) * (1.0 - growth);
   }

   // Null Object resolution at the boundary: callers that do not care get the
   // classic fit-or-native opening rule.
   std::unique_ptr<const IFramingPolicy> framingOrDefault(std::unique_ptr<IFramingPolicy> framing)
   {
      if (framing)
         return framing;

      return std::make_unique<OFitOrNativeFraming>();
   }
}

// -----------------------------------------------------------------------------
// OImageViewport::Impl Definition
// -----------------------------------------------------------------------------
struct OImageViewport::Impl
{
   ObjectIdentity      m_identity {L"ImageViewportObject"};
   ZoomRange           m_range;
   ZoomChangedCallback m_onZoom;

   std::unique_ptr<const IFramingPolicy> m_framing;

   CompositeTransform m_transform {nullptr};
   UIElement          m_content {nullptr};
   UIElement          m_input {nullptr};
   FrameworkElement   m_surface {nullptr};

   Extent m_extent;
   double m_percent {k_nativePercent};
   double m_rotation {0.0};
   double m_panX {0.0};
   double m_panY {0.0};

   bool  m_panning {false};
   bool  m_pendingFrame {false};
   Point m_lastPoint {0.0f, 0.0f};

   UIElement::PointerPressed_revoker      m_pressed {};
   UIElement::PointerMoved_revoker        m_moved {};
   UIElement::PointerReleased_revoker     m_released {};
   UIElement::PointerCaptureLost_revoker  m_lost {};
   UIElement::PointerWheelChanged_revoker m_wheel {};
   FrameworkElement::SizeChanged_revoker  m_resized {};

   Impl(ZoomRange range, std::unique_ptr<const IFramingPolicy> framing) :
      m_range {range},
      m_framing {std::move(framing)}
   {}

   // Device pixels per DIP of the hosting display (1.0 at 96 DPI). Dividing
   // the zoom factor by it makes 100% mean one image pixel per device pixel,
   // not per DIP — "original size" stays true on a scaled display.
   double displayScale(void) const
   {
      const XamlRoot root = m_content ? m_content.XamlRoot() : XamlRoot {nullptr};
      return root ? root.RasterizationScale() : 1.0;
   }

   double scaleFactor(void) const
   {
      return m_percent / (k_nativePercent * displayScale());
   }

   // The drag surface's size in DIPs — the space the pan translation lives in.
   Extent surfaceExtent(void) const
   {
      if (!m_surface)
         return {};

      return {m_surface.ActualWidth(), m_surface.ActualHeight()};
   }

   // The same surface in device pixels, comparable with the image's extent.
   Extent surfacePixels(void) const
   {
      const Extent dips  = surfaceExtent();
      const double scale = displayScale();

      return {dips.width * scale, dips.height * scale};
   }

   void applyScale(void)
   {
      if (!m_transform)
         return;

      m_transform.ScaleX(scaleFactor());
      m_transform.ScaleY(scaleFactor());
   }

   void applyRotation(void)
   {
      if (m_transform)
         m_transform.Rotation(m_rotation);
   }

   void notifyZoom(void) const
   {
      if (m_onZoom)
         m_onZoom(m_percent);
   }

   // The content sits at its Canvas host's origin, so the translation does the
   // centring itself and adds the pan on top. Scale and rotation pivot about
   // the content's centre, so neither moves it — only this translation does.
   void applyTranslation(void)
   {
      if (!m_transform)
         return;

      const Extent surface = surfaceExtent();
      m_transform.TranslateX((surface.width - m_extent.width) / 2.0 + m_panX);
      m_transform.TranslateY((surface.height - m_extent.height) / 2.0 + m_panY);
   }

   // Keep the view inside the image: clamp the pan so no image edge ever
   // crosses into the viewport. The image's pixel extent times the scale
   // factor is its on-screen size in DIPs — the space the viewport measures in.
   void clampPan(void)
   {
      const PanBounds bounds = makePanBounds(m_extent, scaleFactor(), m_rotation, surfaceExtent());

      m_panX = bounds.clampX(m_panX);
      m_panY = bounds.clampY(m_panY);
      applyTranslation();
   }

   void clearPan(void)
   {
      m_panX = 0.0;
      m_panY = 0.0;
      applyTranslation();
   }

   // Same bounds the pan clamps against: if they permit any travel, the image
   // overflows its surface and a drag would move it.
   bool isContentDraggable(void) const
   {
      return makePanBounds(m_extent, scaleFactor(), m_rotation, surfaceExtent()).allowsPan();
   }

   void setZoom(double percent)
   {
      m_percent = std::clamp(percent, m_range.min, m_range.max);
      applyScale();
      clampPan();
      notifyZoom();
   }

   void rotateClockwise(void)
   {
      m_rotation = std::fmod(m_rotation + k_quarterTurn, k_fullTurn);
      applyRotation();
      clampPan();
   }

   // Frame the adopted content: pan recentred, rotation cleared, and the zoom
   // chosen by the framing policy — native size, or scaled down to fit an
   // oversized image. Before first layout the framing is retried on resize.
   void frame(void)
   {
      const Extent viewport = surfacePixels();

      m_pendingFrame = viewport.isEmpty();
      m_rotation     = 0.0;

      clearPan();
      applyRotation();
      setZoom(m_framing->zoomPercentFor(m_extent, viewport));
   }

   void frameContent(const Extent& nativeSize)
   {
      m_extent = nativeSize;
      frame();
   }

   void bind(const UIElement& content, const UIElement& input)
   {
      m_content = content;
      m_input   = input;
      m_surface = input.try_as<FrameworkElement>();

      bindTransform();
      wirePan();
      wireWheel();
      wireResize();
   }

   // Pivot scale and rotation about the content's centre; the translation
   // (centring + pan) is applied last and is unaffected by the origin.
   void bindTransform(void)
   {
      m_transform = CompositeTransform {};
      m_content.RenderTransformOrigin(Point {0.5f, 0.5f});
      m_content.RenderTransform(m_transform);
      applyScale();
      applyRotation();
      applyTranslation();
   }

   void wirePan(void)
   {
      m_pressed  = m_input.PointerPressed(winrt::auto_revoke, {this, &Impl::onPointerPressed});
      m_moved    = m_input.PointerMoved(winrt::auto_revoke, {this, &Impl::onPointerMoved});
      m_released = m_input.PointerReleased(winrt::auto_revoke, {this, &Impl::onPointerReleased});
      m_lost     = m_input.PointerCaptureLost(winrt::auto_revoke, {this, &Impl::onPointerCaptureLost});
   }

   void wireWheel(void)
   {
      m_wheel = m_input.PointerWheelChanged(winrt::auto_revoke, {this, &Impl::onPointerWheel});
   }

   void wireResize(void)
   {
      if (m_surface)
         m_resized = m_surface.SizeChanged(winrt::auto_revoke, {this, &Impl::onSurfaceResized});
   }

   // A framing requested before the surface had a layout completes now; any
   // later resize only re-clamps the pan so the view stays inside the image.
   void onSurfaceResized(const IInspectable&, const SizeChangedEventArgs&)
   {
      if (m_pendingFrame)
         frame();
      else
         clampPan();
   }

   // Start a drag only on the left button, capturing the pointer so moves keep
   // arriving even if the cursor leaves the surface.
   void onPointerPressed(const IInspectable&, const PointerRoutedEventArgs& args)
   {
      const auto point = args.GetCurrentPoint(m_input);
      if (!point.Properties().IsLeftButtonPressed())
         return;

      m_panning   = m_input.CapturePointer(args.Pointer());
      m_lastPoint = point.Position();
      args.Handled(true);
   }

   void onPointerMoved(const IInspectable&, const PointerRoutedEventArgs& args)
   {
      if (!m_panning)
         return;

      const Point pos = args.GetCurrentPoint(m_input).Position();
      panBy(pos.X - m_lastPoint.X, pos.Y - m_lastPoint.Y);
      m_lastPoint = pos;
      args.Handled(true);
   }

   // Accumulate the drag delta into the pan, then clamp it so the drag stops
   // dead at the image's edges instead of revealing what lies beyond.
   void panBy(double deltaX, double deltaY)
   {
      m_panX += deltaX;
      m_panY += deltaY;
      clampPan();
   }

   // One wheel notch scales by the step factor, anchored on the cursor; a
   // smooth wheel's fractional notches scale by the matching fraction.
   void onPointerWheel(const IInspectable&, const PointerRoutedEventArgs& args)
   {
      const auto point  {args.GetCurrentPoint(m_input)};
      const double turn {point.Properties().MouseWheelDelta() / k_wheelNotch};

      zoomTowards(point.Position(), std::pow(k_wheelZoomStep, turn));
      args.Handled(true);
   }

   // Zoom about 'anchor': scale first, then shift the pan by the anchor's
   // drift from the content's centre (at surface-centre + pan) so the image
   // point under the cursor stays put — clamped like any other pan move.
   void zoomTowards(const Point& anchor, double factor)
   {
      const double before {scaleFactor()};
      setZoom(m_percent * factor);

      const double growth  {scaleFactor() / before};
      const Extent surface {surfaceExtent()};
      panBy(anchorDrift(anchor.X, surface.width / 2.0 + m_panX, growth),
            anchorDrift(anchor.Y, surface.height / 2.0 + m_panY, growth));
   }

   void onPointerReleased(const IInspectable&, const PointerRoutedEventArgs& args)
   {
      if (!m_panning)
         return;

      m_panning = false;
      m_input.ReleasePointerCapture(args.Pointer());
      args.Handled(true);
   }

   // Capture can be stolen (window deactivation, another gesture); just stop
   // panning — the capture is already gone, so nothing to release.
   void onPointerCaptureLost(const IInspectable&, const PointerRoutedEventArgs&)
   {
      m_panning = false;
   }
};

// -----------------------------------------------------------------------------
// OImageViewport Implementation
// -----------------------------------------------------------------------------
OImageViewport::OImageViewport(ZoomRange range, std::unique_ptr<IFramingPolicy> framing) :
   m_pImpl {std::make_unique<Impl>(range, framingOrDefault(std::move(framing)))}
{}
// -----------------------------------------------------------------------------
OImageViewport::~OImageViewport() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OImageViewport::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OImageViewport::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImageViewport::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OImageViewport::attach(const UIElement& content, const UIElement& inputSurface)
{
   m_pImpl->bind(content, inputSurface);
}
// -----------------------------------------------------------------------------
void OImageViewport::frameContent(const Extent& nativeSize)
{
   m_pImpl->frameContent(nativeSize);
}
// -----------------------------------------------------------------------------
void OImageViewport::setZoom(double percent)
{
   m_pImpl->setZoom(percent);
}
// -----------------------------------------------------------------------------
void OImageViewport::zoomBy(double factor)
{
   m_pImpl->setZoom(m_pImpl->m_percent * factor);
}
// -----------------------------------------------------------------------------
double OImageViewport::zoom(void) const
{
   return m_pImpl->m_percent;
}
// -----------------------------------------------------------------------------
void OImageViewport::rotateClockwise(void)
{
   m_pImpl->rotateClockwise();
}
// -----------------------------------------------------------------------------
double OImageViewport::rotation(void) const
{
   return m_pImpl->m_rotation;
}
// -----------------------------------------------------------------------------
bool OImageViewport::isContentDraggable(void) const
{
   return m_pImpl->isContentDraggable();
}
// -----------------------------------------------------------------------------
void OImageViewport::reset(void)
{
   m_pImpl->frame();
}
// -----------------------------------------------------------------------------
void OImageViewport::onZoomChanged(ZoomChangedCallback callback)
{
   m_pImpl->m_onZoom = std::move(callback);
}
// -----------------------------------------------------------------------------
