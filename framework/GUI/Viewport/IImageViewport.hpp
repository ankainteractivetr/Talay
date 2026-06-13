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

#ifndef ANKA_GUI_VIEWPORT_IIMAGEVIEWPORT_HPP
#define ANKA_GUI_VIEWPORT_IIMAGEVIEWPORT_HPP

#include <winrt/Microsoft.UI.Xaml.h>

#include <functional>

#include "Extent.hpp"

#include "framework/Core/Object/IObject.hpp"

namespace anka::GUI::Viewport
{
   // Observer hook fired whenever the zoom factor changes — from any source
   // (zoom buttons, slider, or a fit-reset). The host uses it to keep its zoom
   // readout in sync without the viewport ever knowing about a slider or label.
   using ZoomChangedCallback = std::function<void(double /*percent*/)>;

   /*
   * Abstract Image Viewport Interface
   *
   * Owns the on-screen transform of a single displayed image: scale (zoom),
   * rotation, and pan offset. The host binds two XAML elements once via attach()
   * — the content it transforms and the surface it listens on for drag and wheel
   * gestures — hands each freshly shown image to frameContent(), then drives
   * zoom/rotation through this interface and lets panning and wheel-zooming
   * happen on their own, clamped so the view never leaves the image. The
   * concrete WinUI machinery (render transform, pointer capture) is hidden
   * behind the implementation, so callers depend only on this abstraction.
   *
   */
   class IImageViewport : public anka::Core::Object::IObject
   {
      public:

         virtual ~IImageViewport() noexcept override = default;

         // Bind the transform to 'content' and route drag-to-pan from
         // 'inputSurface'. 'content' must sit at the origin of a non-clipping
         // host (a Canvas) filling 'inputSurface' — any other panel layout-clips
         // content bigger than its slot; the transform does the centring.
         // Idempotent: a second call rebinds to new elements.
         virtual void attach(const winrt::Microsoft::UI::Xaml::UIElement& content,
                             const winrt::Microsoft::UI::Xaml::UIElement& inputSurface) = 0;

         // Adopt 'nativeSize' — the shown image's extent in device pixels — and
         // frame it: opened at native size when it fits the viewport, scaled
         // down to fit when it overflows. The zoom callback reports the result.
         virtual void frameContent(const Extent& nativeSize) = 0;

         // Absolute zoom in percent (100 == native size), clamped to the range.
         virtual void setZoom(double percent) = 0;

         // Relative zoom: multiplies the current factor (e.g. 1.15 to zoom in).
         virtual void zoomBy(double factor) = 0;

         virtual double zoom(void) const = 0;

         // Quarter-turn clockwise, normalized into [0, 360).
         virtual void rotateClockwise(void) = 0;

         virtual double rotation(void) const = 0;

         // True when the framed image overflows the viewport on some axis, so a
         // drag would actually pan it — what a cursor consults to show the move
         // affordance only while there is somewhere to drag to.
         virtual bool isContentDraggable(void) const = 0;

         // Re-frame the current content from scratch: pan recentred, rotation
         // cleared, zoom back to the framing rule's choice — as if the image
         // had just been opened.
         virtual void reset(void) = 0;

         virtual void onZoomChanged(ZoomChangedCallback callback) = 0;
   };
}

#endif // ANKA_GUI_VIEWPORT_IIMAGEVIEWPORT_HPP
