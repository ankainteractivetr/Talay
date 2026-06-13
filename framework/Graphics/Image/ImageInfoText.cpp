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
#include "ImageInfoText.hpp"

#include <format>
#include <iterator>
#include <numeric>

namespace anka::Graphics::Image::ImageInfoText
{
   namespace
   {
      const std::wstring k_dash {L"—"};

      // Scale a non-zero byte count to the largest unit that keeps it readable.
      std::wstring scaleBytes(std::uint64_t bytes)
      {
         static constexpr const wchar_t* units[] {L"B", L"KB", L"MB", L"GB"};
         double      value {static_cast<double>(bytes)};
         std::size_t unit  {0};

         while (value >= 1024.0 && unit + 1 < std::size(units)) {
            value /= 1024.0;
            ++unit;
         }

         if (unit == 0)
            return std::format(L"{} {}", bytes, units[unit]);

         return std::format(L"{:.1f} {}", value, units[unit]);
      }
   }

   std::wstring orDash(const std::wstring& value)
   {
      return value.empty() ? k_dash : value;
   }

   std::wstring dimensions(const ImageMetadata& meta)
   {
      if (!meta.valid())
         return k_dash;

      return std::format(L"{} × {}", meta.width, meta.height);
   }

   std::wstring pixelDimensions(const ImageMetadata& meta)
   {
      if (!meta.valid())
         return k_dash;

      return std::format(L"{} × {} px", meta.width, meta.height);
   }

   std::wstring kilobytes(std::uint64_t bytes)
   {
      if (bytes == 0)
         return k_dash;

      const std::uint64_t kib {(bytes + 1023) / 1024};
      return std::format(L"{} KB", kib);
   }

   std::wstring fileSize(std::uint64_t bytes)
   {
      if (bytes == 0)
         return k_dash;

      return scaleBytes(bytes);
   }

   std::wstring aspectRatio(const ImageMetadata& meta)
   {
      if (!meta.valid())
         return k_dash;

      const std::uint32_t divisor {std::gcd(meta.width, meta.height)};
      return std::format(L"{}:{}", meta.width / divisor, meta.height / divisor);
   }

   std::wstring colorDepth(const ImageMetadata& meta)
   {
      if (meta.bitDepth == 0)
         return k_dash;

      return std::format(L"{}-bit {}", meta.bitDepth, meta.pixelFormat);
   }

   std::wstring resolution(const ImageMetadata& meta)
   {
      if (meta.dpiX <= 0.0 || meta.dpiY <= 0.0)
         return k_dash;

      return std::format(L"{:.0f} × {:.0f} DPI", meta.dpiX, meta.dpiY);
   }
}
