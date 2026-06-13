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

#ifndef ANKA_GUI_VIEWPORT_OIMAGEVIEWPORT_HPP
#define ANKA_GUI_VIEWPORT_OIMAGEVIEWPORT_HPP

#include "IImageViewport.hpp"
#include "IFramingPolicy.hpp"

#include <memory>

namespace anka::GUI::Viewport
{
   // The zoom limits the viewport will clamp to, in percent. Defaults mirror the
   // toolbar slider's range; inject a different range to widen or narrow it
   // without touching the implementation (Open/Closed).
   struct ZoomRange
   {
      double min {10.0};
      double max {400.0};
   };

   /*
   *
   * Concrete Image Viewport Class
   *
   * A facade over a WinUI CompositeTransform: zoom maps to scale and rotation to
   * the transform's Rotation, both about the content's centre; the translation
   * centres the Canvas-hosted content in the surface and adds the pan on top.
   * Each framed image opens at the injected framing policy's zoom —
   * fit-or-native by default — with 100% meaning one image pixel per device
   * pixel. Drag-to-pan is implemented by capturing the pointer on the bound
   * input surface and accumulating the move delta into the translation, clamped
   * (as every zoom, rotation and resize is) so the view never leaves the image.
   * The wheel zooms about the cursor: each notch multiplies the zoom by the
   * toolbar's step, with the pan compensated so the image point under the
   * cursor stays fixed. One instance lives per displayed viewport (not a singleton — a window owns
   * it). All WinUI state and the event subscriptions are hidden behind a pimpl,
   * whose RAII revokers detach the handlers when the viewport dies.
   *
   */
   class OImageViewport final : public IImageViewport
   {
      public:

         explicit OImageViewport(ZoomRange range = {},
                                 std::unique_ptr<IFramingPolicy> framing = nullptr);

         OImageViewport(const OImageViewport& viewport) = delete;
         OImageViewport(OImageViewport&& viewport) noexcept = delete;

         ~OImageViewport() noexcept override;

         OImageViewport& operator=(const OImageViewport& viewport) = delete;
         OImageViewport& operator=(OImageViewport&& viewport) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IImageViewport overrides
         //

         void attach(const winrt::Microsoft::UI::Xaml::UIElement& content,
                     const winrt::Microsoft::UI::Xaml::UIElement& inputSurface) override;

         void frameContent(const Extent& nativeSize) override;

         void   setZoom(double percent) override;
         void   zoomBy(double factor) override;
         double zoom(void) const override;

         void   rotateClockwise(void) override;
         double rotation(void) const override;

         bool isContentDraggable(void) const override;

         void reset(void) override;

         void onZoomChanged(ZoomChangedCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_VIEWPORT_OIMAGEVIEWPORT_HPP
