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

#ifndef ANKA_SYSTEM_CLIPBOARD_OIMAGECLIPBOARD_HPP
#define ANKA_SYSTEM_CLIPBOARD_OIMAGECLIPBOARD_HPP

#include "IImageClipboard.hpp"

#include <memory>

namespace anka::System::Clipboard
{
   /*
   *
   * Concrete Image Clipboard Class (Singleton)
   *
   * Pure Windows-API implementation: user32 clipboard calls fed with
   * global-memory DIB payloads. The image is published twice from one pixel
   * buffer - CF_DIBV5 keeps the alpha channel for transparency-aware
   * targets, CF_DIB keeps legacy targets (Paint, Office) pasting. Stateless
   * by nature, so a single instance per process suffices, matching the
   * other framework system services. Win32 details stay behind a pimpl.
   *
   */
   class OImageClipboard final : public IImageClipboard
   {
      public:

         OImageClipboard(const OImageClipboard& object) = delete;
         OImageClipboard(OImageClipboard&& object) noexcept = delete;

         ~OImageClipboard() noexcept override;

         OImageClipboard& operator=(const OImageClipboard& object) = delete;
         OImageClipboard& operator=(OImageClipboard&& object) noexcept = delete;

         static OImageClipboard& getInstance();

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IImageClipboard overrides
         //

         bool copyImage(const Graphics::Image::DecodedImage& image, HWND owner) const override;

      private:

         // Singleton COTOR
         OImageClipboard();

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_SYSTEM_CLIPBOARD_OIMAGECLIPBOARD_HPP
