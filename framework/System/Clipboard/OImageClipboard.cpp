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
#include "OImageClipboard.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cstring>
#include <memory>
#include <span>
#include <vector>

#pragma comment(lib, "User32.lib")   // OpenClipboard / SetClipboardData

using namespace anka::System::Clipboard;
using anka::Core::Object::ObjectIdentity;
using anka::Graphics::Image::DecodedImage;

namespace
{
   // Deleter so an unpublished DIB payload rides in a unique_ptr and is freed
   // on every path; release() hands ownership to the clipboard on success.
   struct GlobalMemoryDeleter
   {
      void operator()(HGLOBAL memory) const
      {
         GlobalFree(memory);
      }
   };
   using GlobalMemory = std::unique_ptr<void, GlobalMemoryDeleter>;

   // RAII frame around one clipboard transaction: open and take ownership by
   // emptying, with a guaranteed close. Another process may hold the clipboard
   // for a moment, so opening retries briefly before reporting failure.
   class ClipboardSession
   {
      public:

         explicit ClipboardSession(HWND owner) :
            m_open {openWithRetry(owner)}
         {
            if (m_open)
               EmptyClipboard();
         }

         ClipboardSession(const ClipboardSession& session) = delete;
         ClipboardSession& operator=(const ClipboardSession& session) = delete;

         ~ClipboardSession()
         {
            if (m_open)
               CloseClipboard();
         }

         bool isOpen(void) const
         {
            return m_open;
         }

      private:

         static bool openWithRetry(HWND owner)
         {
            for (int attempt {0}; attempt < 5; ++attempt) {
               if (OpenClipboard(owner))
                  return true;

               Sleep(20);
            }

            return false;
         }

         bool m_open;
   };
}

// -----------------------------------------------------------------------------
// OImageClipboard::Impl Definition
// -----------------------------------------------------------------------------
struct OImageClipboard::Impl
{
   ObjectIdentity m_identity {L"ImageClipboardObject"};

   static constexpr std::uint32_t k_bytesPerPixel {4};

   // Pixel rows are 32-bit, so they already sit on the DWORD boundary a
   // packed DIB requires - no padding bookkeeping anywhere below.
   static std::size_t rowBytes(const DecodedImage& image)
   {
      return std::size_t {image.width()} * k_bytesPerPixel;
   }

   // Clipboard DIBs carry straight alpha and bottom-up rows, while a
   // DecodedImage is premultiplied and top-down: one packed buffer bridges
   // both conventions and then feeds every published format.
   static std::vector<std::byte> packPixels(const DecodedImage& image)
   {
      std::vector<std::byte> packed(rowBytes(image) * image.height());

      for (std::uint32_t row {0}; row < image.height(); ++row)
         packRow(packed, image, row);

      return packed;
   }

   // Copies one source row into its mirrored (bottom-up) slot, dropping any
   // stride padding, then straightens the alpha in place.
   static void packRow(std::vector<std::byte>& packed, const DecodedImage& image, std::uint32_t row)
   {
      const auto* source {static_cast<const std::byte*>(image.pixels()) + std::size_t {row} * image.stride()};
      std::byte*  target {packed.data() + rowBytes(image) * (image.height() - 1 - row)};

      std::memcpy(target, source, rowBytes(image));
      unpremultiplyRow(target, image.width());
   }

   // Premultiplied -> straight alpha: scale each colour channel back up by
   // the alpha it was multiplied with. Fully opaque and fully transparent
   // pixels are already straight, which keeps the common case free.
   static void unpremultiplyRow(std::byte* row, std::uint32_t width)
   {
      for (std::uint32_t x {0}; x < width; ++x, row += k_bytesPerPixel) {
         const auto alpha {std::to_integer<std::uint32_t>(row[3])};

         if (alpha != 0 && alpha != 255)
            straightenPixel(row, alpha);
      }
   }

   static void straightenPixel(std::byte* pixel, std::uint32_t alpha)
   {
      for (int channel {0}; channel < 3; ++channel) {
         const std::uint32_t value {std::to_integer<std::uint32_t>(pixel[channel]) * 255u / alpha};
         pixel[channel] = static_cast<std::byte>((std::min)(value, 255u));
      }
   }

   // The V5 header is what carries transparency: BI_BITFIELDS plus an
   // explicit alpha mask tells paste targets the fourth byte is alpha.
   // Positive height marks the rows as bottom-up, the DIB default.
   static BITMAPV5HEADER makeV5Header(const DecodedImage& image)
   {
      return {
         .bV5Size        = sizeof(BITMAPV5HEADER),
         .bV5Width       = static_cast<LONG>(image.width()),
         .bV5Height      = static_cast<LONG>(image.height()),
         .bV5Planes      = 1,
         .bV5BitCount    = 32,
         .bV5Compression = BI_BITFIELDS,
         .bV5SizeImage   = static_cast<DWORD>(rowBytes(image) * image.height()),
         .bV5RedMask     = 0x00FF0000,
         .bV5GreenMask   = 0x0000FF00,
         .bV5BlueMask    = 0x000000FF,
         .bV5AlphaMask   = 0xFF000000,
         .bV5CSType      = LCS_sRGB,
         .bV5Intent      = LCS_GM_IMAGES};
   }

   // The classic header keeps alpha-blind targets pasting; 32-bit BI_RGB is
   // the layout every legacy consumer understands.
   static BITMAPINFOHEADER makeDibHeader(const DecodedImage& image)
   {
      return {
         .biSize        = sizeof(BITMAPINFOHEADER),
         .biWidth       = static_cast<LONG>(image.width()),
         .biHeight      = static_cast<LONG>(image.height()),
         .biPlanes      = 1,
         .biBitCount    = 32,
         .biCompression = BI_RGB,
         .biSizeImage   = static_cast<DWORD>(rowBytes(image) * image.height())};
   }

   // One contiguous global block, header bytes first and pixel rows after -
   // exactly the packed-DIB layout clipboard consumers expect.
   template <typename Header>
   static GlobalMemory toGlobal(const Header& header, std::span<const std::byte> pixels)
   {
      GlobalMemory memory {GlobalAlloc(GMEM_MOVEABLE, sizeof(Header) + pixels.size())};

      if (memory && fill(memory.get(), &header, sizeof(Header), pixels))
         return memory;

      return {};
   }

   // Lock, lay out header then pixels, unlock - false when the block cannot
   // be locked, so the caller's guard frees it.
   static bool fill(HGLOBAL memory, const void* header, std::size_t headerSize, std::span<const std::byte> pixels)
   {
      auto* target {static_cast<std::byte*>(GlobalLock(memory))};
      if (!target)
         return false;

      std::memcpy(target, header, headerSize);
      std::memcpy(target + headerSize, pixels.data(), pixels.size());
      GlobalUnlock(memory);

      return true;
   }

   // SetClipboardData owns the payload only on success, so the guard lets go
   // exactly then; a failed put frees the block on scope exit instead.
   static bool publish(UINT format, GlobalMemory payload)
   {
      if (!payload)
         return false;

      if (!SetClipboardData(format, payload.get()))
         return false;

      payload.release();
      return true;
   }

   // CF_DIBV5 keeps transparency for targets that honour it; CF_DIB keeps
   // alpha-blind targets pasting. One landed format already means pastable.
   static bool publishFormats(const DecodedImage& image, std::span<const std::byte> pixels)
   {
      const bool fullColour {publish(CF_DIBV5, toGlobal(makeV5Header(image), pixels))};
      const bool legacy     {publish(CF_DIB,   toGlobal(makeDibHeader(image), pixels))};

      return fullColour || legacy;
   }

   bool copyImage(const DecodedImage& image, HWND owner) const
   {
      if (image.empty())
         return false;

      const std::vector<std::byte> pixels {packPixels(image)};

      const ClipboardSession session {owner};
      if (!session.isOpen())
         return false;

      return publishFormats(image, pixels);
   }
};

// -----------------------------------------------------------------------------
// OImageClipboard Implementation
// -----------------------------------------------------------------------------
OImageClipboard::OImageClipboard() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OImageClipboard::~OImageClipboard() noexcept = default;
// -----------------------------------------------------------------------------
OImageClipboard& OImageClipboard::getInstance()
{
   static OImageClipboard s_instance;
   return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OImageClipboard::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OImageClipboard::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImageClipboard::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OImageClipboard::copyImage(const DecodedImage& image, HWND owner) const
{
   return m_pImpl->copyImage(image, owner);
}
// -----------------------------------------------------------------------------
