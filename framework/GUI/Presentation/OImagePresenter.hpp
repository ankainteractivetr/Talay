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

#ifndef ANKA_GUI_PRESENTATION_OIMAGEPRESENTER_HPP
#define ANKA_GUI_PRESENTATION_OIMAGEPRESENTER_HPP

#include "IImagePresenter.hpp"

#include <memory>

namespace anka::GUI::Presentation
{
   /*
   *
   * Concrete Image Presenter Class
   *
   * Drives a single XAML Image: it creates a surface sized to the decoded image,
   * uploads its frames through the surface-bitmap factory, draws the visible frame
   * 1:1 into the surface and copies it into a WriteableBitmap the Image shows. An
   * animation additionally gets a DispatcherTimer keyed to each frame's delay,
   * re-blitting on every advance. The renderer, resource keeper and bitmap factory
   * it relies on are shared singletons it merely consumes; everything it owns (the
   * surface id, the current bitmap, the canvas, the timer) is hidden behind a
   * pimpl. One instance lives per displayed image surface (not a singleton — a
   * window owns it).
   *
   */
   class OImagePresenter final : public IImagePresenter
   {
      public:

         OImagePresenter();

         OImagePresenter(const OImagePresenter& presenter) = delete;
         OImagePresenter(OImagePresenter&& presenter) noexcept = delete;

         ~OImagePresenter() noexcept override;

         OImagePresenter& operator=(const OImagePresenter& presenter) = delete;
         OImagePresenter& operator=(OImagePresenter&& presenter) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IImagePresenter overrides
         //

         void attach(const winrt::Microsoft::UI::Xaml::Controls::Image& target) override;

         bool present(const anka::Graphics::Image::ImageSequence& sequence) override;

         void clear(void) override;

         void setEffect(std::shared_ptr<const anka::Graphics::Effect::IPixelEffect> effect) override;

         winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap canvas(void) const override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_PRESENTATION_OIMAGEPRESENTER_HPP
