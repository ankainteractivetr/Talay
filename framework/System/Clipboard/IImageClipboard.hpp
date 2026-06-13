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

#ifndef ANKA_SYSTEM_CLIPBOARD_IIMAGECLIPBOARD_HPP
#define ANKA_SYSTEM_CLIPBOARD_IIMAGECLIPBOARD_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Image/DecodedImage.hpp"
#include "framework/System/Windows/windows.hpp"

namespace anka::System::Clipboard
{
   /*
   * Abstract Image Clipboard Interface (Facade)
   *
   * One call hides the whole Windows clipboard handshake: opening and
   * emptying the clipboard, building the bitmap payloads and handing their
   * ownership to the system. Callers give decoded pixels and learn only
   * whether the image is now pastable elsewhere, so no clipboard format or
   * global-memory detail ever leaks above this seam.
   *
   */
   class IImageClipboard : public anka::Core::Object::IObject
   {
      public:

         virtual ~IImageClipboard() noexcept override = default;

         // Replaces the clipboard content with 'image', published in the
         // formats paste targets expect. 'owner' is the window the clipboard
         // session is opened on behalf of. False when the clipboard was
         // unavailable or the payload could not be built.
         virtual bool copyImage(const Graphics::Image::DecodedImage& image, HWND owner) const = 0;
   };
}

#endif // ANKA_SYSTEM_CLIPBOARD_IIMAGECLIPBOARD_HPP
