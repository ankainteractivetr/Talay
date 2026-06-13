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

#ifndef ANKA_GUI_PRESENTATION_IIMAGEPRESENTER_HPP
#define ANKA_GUI_PRESENTATION_IIMAGEPRESENTER_HPP

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

#include "framework/Core/Object/IObject.hpp"

#include <memory>

// Forward-declared: the sequence is only referenced here; the full type is pulled
// in by the implementation that uploads and draws its frames.
namespace anka::Graphics::Image { class ImageSequence; }

// Forward-declared: the effect is only held here; the implementation that runs
// it over each frame pulls in the full type.
namespace anka::Graphics::Effect { class IPixelEffect; }

namespace anka::GUI::Presentation
{
   /*
   * Abstract Image Presenter Interface
   *
   * Presents one decoded image — still or animation — inside a XAML Image element.
   * It owns the Direct2D surface its frames live on, the WriteableBitmap they are
   * copied into, and (only for an animation) the timer that plays them back. The
   * host binds the target Image once via attach(), then hands each freshly decoded
   * sequence to present(); whether it is a still or a GIF, and all the WinUI /
   * Direct2D machinery, stay hidden behind the implementation, so callers depend
   * only on this abstraction.
   *
   */
   class IImagePresenter : public anka::Core::Object::IObject
   {
      public:

         virtual ~IImagePresenter() noexcept override = default;

         // Bind the Image element the presenter draws into. Idempotent: a second
         // call rebinds to a new element.
         virtual void attach(const winrt::Microsoft::UI::Xaml::Controls::Image& target) = 0;

         // Show 'sequence', replacing whatever was on screen. Returns false — and
         // leaves nothing shown — for an empty or unuploadable image.
         virtual bool present(const anka::Graphics::Image::ImageSequence& sequence) = 0;

         // Stop any playback and release the current image and its surface.
         virtual void clear(void) = 0;

         // Apply 'effect' to every frame shown from now on — including the one
         // on screen, which redraws immediately; null restores the original
         // pixels. View-time only: the decoded image is never modified, and
         // present()/clear() drop any active effect, so each image opens clean.
         virtual void setEffect(std::shared_ptr<const anka::Graphics::Effect::IPixelEffect> effect) = 0;

         // The bitmap the current frame is drawn into, for reuse as a preview
         // elsewhere; null while nothing is shown.
         virtual winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap canvas(void) const = 0;
   };
}

#endif // ANKA_GUI_PRESENTATION_IIMAGEPRESENTER_HPP
