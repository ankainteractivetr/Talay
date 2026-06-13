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

#ifndef ANKA_GRAPHICS_IMAGE_IMAGEINFOTEXT_HPP
#define ANKA_GRAPHICS_IMAGE_IMAGEINFOTEXT_HPP

#include "ImageMetadata.hpp"

#include <cstdint>
#include <string>

namespace anka::Graphics::Image::ImageInfoText
{
   // Pure ImageMetadata → display-string mappers for the "file info" surfaces (modal
   // rows, toolbar chips, viewport HUD). They turn the reader's raw numbers into
   // exactly what each field shows, returning an em-dash placeholder for missing
   // data so a panel never reads blank. Deliberately winrt-free (plain std::wstring)
   // so the formatting logic — the part with real branching and arithmetic — stays
   // in the framework layer, reusable and unit-testable; the view just wraps each
   // result in an hstring at the assignment site.

   // 'value' itself, or the em-dash placeholder when it is empty.
   std::wstring orDash(const std::wstring& value);

   // Bare "W × H" for the toolbar chip and the HUD readout; em-dash if invalid.
   std::wstring dimensions(const ImageMetadata& meta);

   // "W × H px" for the modal; em-dash if invalid.
   std::wstring pixelDimensions(const ImageMetadata& meta);

   // Whole kilobytes (1 KB == 1024 B), rounded up so a non-empty file never reads
   // "0 KB"; em-dash for an empty file.
   std::wstring kilobytes(std::uint64_t bytes);

   // The largest sensible unit (B/KB/MB/GB) for the modal; em-dash for an empty file.
   std::wstring fileSize(std::uint64_t bytes);

   // Reduced aspect ratio "W:H"; em-dash if invalid.
   std::wstring aspectRatio(const ImageMetadata& meta);

   // "N-bit FORMAT"; em-dash when the depth is unknown.
   std::wstring colorDepth(const ImageMetadata& meta);

   // "X × Y DPI"; em-dash when the resolution is unknown.
   std::wstring resolution(const ImageMetadata& meta);
}

#endif // ANKA_GRAPHICS_IMAGE_IMAGEINFOTEXT_HPP
