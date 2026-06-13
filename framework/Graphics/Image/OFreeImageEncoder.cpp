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
#include "OFreeImageEncoder.hpp"

#include "FreeImage.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <utility>

using namespace anka::Graphics::Image;


// FreeImage plumbing, file-local so FreeImage.h never leaks beyond this unit.
namespace
{
	// RAII over an FIBITMAP* (Effective C++ Item 13): the handle frees itself on
	// scope exit, so every early return below stays leak-free.
	struct DibDeleter
	{
		void operator()(FIBITMAP* dib) const noexcept
		{
			FreeImage_Unload(dib);
		}
	};

	using ScopedDib = std::unique_ptr<FIBITMAP, DibDeleter>;

	// Recover a straight-alpha channel value from a premultiplied one
	// (c = c_premul * 255 / a), rounded and clamped. a == 0 yields 0.
	std::uint8_t unpremultiply(std::uint8_t channel, std::uint8_t alpha)
	{
		if (alpha == 0) {
			return 0;
		}

		const unsigned value = (channel * 255u + alpha / 2u) / alpha;
		return static_cast<std::uint8_t>(value < 255u ? value : 255u);
	}

	// Copy one BGRA scanline, undoing premultiplication. Source and destination are
	// both byte-order BGRA; only the colour channels are scaled, alpha is verbatim.
	void copyScanline(BYTE* dst, const std::byte* src, std::uint32_t width)
	{
		for (std::uint32_t x = 0; x < width; ++x, dst += 4, src += 4) {
			const auto* in = reinterpret_cast<const std::uint8_t*>(src);
			dst[FI_RGBA_BLUE]  = unpremultiply(in[0], in[3]);
			dst[FI_RGBA_GREEN] = unpremultiply(in[1], in[3]);
			dst[FI_RGBA_RED]   = unpremultiply(in[2], in[3]);
			dst[FI_RGBA_ALPHA] = in[3];
		}
	}

	// Build a 32-bit FreeImage bitmap from the decoded pixels. DecodedImage is
	// top-down while FreeImage is bottom-up, so source row y lands on scanline
	// (height-1-y) — that single index flips the image as it is copied.
	ScopedDib buildBitmap(const DecodedImage& image)
	{
		const std::uint32_t width  = image.width();
		const std::uint32_t height = image.height();
		ScopedDib dib{ FreeImage_Allocate(static_cast<int>(width), static_cast<int>(height), 32) };
		if (!dib) {
			return ScopedDib{ nullptr };
		}

		const auto* src = static_cast<const std::byte*>(image.pixels());
		for (std::uint32_t y = 0; y < height; ++y) {
			copyScanline(FreeImage_GetScanLine(dib.get(), static_cast<int>(height - 1 - y)),
			             src + static_cast<std::size_t>(y) * image.stride(), width);
		}
		return dib;
	}

	// Reduce the bitmap to the deepest pixel format the target codec can export:
	// keep 32-bit where supported, else drop alpha to 24-bit (JPEG, plain BMP, ...),
	// else quantise to an 8-bit palette (GIF). The codec answers what it supports,
	// so no format is hard-coded here.
	ScopedDib fitToFormat(ScopedDib dib, FREE_IMAGE_FORMAT format)
	{
		if (FreeImage_FIFSupportsExportBPP(format, 32)) {
			return dib;
		}

		ScopedDib rgb{ FreeImage_ConvertTo24Bits(dib.get()) };
		if (!rgb || FreeImage_FIFSupportsExportBPP(format, 24)) {
			return rgb;
		}

		return ScopedDib{ FreeImage_ColorQuantize(rgb.get(), FIQ_WUQUANT) };
	}

	// FreeImage chooses the codec from a filename, so an extension is turned into a
	// throwaway one ("thumb.<ext>"); the leading dot is added when the caller omitted
	// it, so both ".PNG" and "png" map the same way.
	std::wstring synthesizeFilename(const std::wstring& extension)
	{
		const std::wstring dotted = (!extension.empty() && extension.front() == L'.') ? extension : (L"." + extension);
		return L"thumb" + dotted;
	}

	// FreeImage's format names are ASCII (FreeImage_GetFormatFromFIF), so a plain
	// char-by-char widen is exact here.
	std::wstring widen(const char* text)
	{
		return text ? std::wstring(text, text + std::strlen(text)) : std::wstring{};
	}
}


// -----------------------------------------------------------------------------
// OFreeImageEncoder Implementation
// -----------------------------------------------------------------------------
bool OFreeImageEncoder::encode(const DecodedImage& image, const std::wstring& path) const
{
	if (image.empty()) {
		return false;
	}

	const FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilenameU(path.c_str());
	if (format == FIF_UNKNOWN || !FreeImage_FIFSupportsWriting(format)) {
		return false;
	}

	ScopedDib dib = fitToFormat(buildBitmap(image), format);
	return dib && FreeImage_SaveU(format, dib.get(), path.c_str(), 0) != FALSE;
}
// -----------------------------------------------------------------------------
std::wstring OFreeImageEncoder::encodableFormatKey(const std::wstring& extension) const
{
	const std::wstring filename = synthesizeFilename(extension);
	const FREE_IMAGE_FORMAT format = FreeImage_GetFIFFromFilenameU(filename.c_str());
	if (format == FIF_UNKNOWN || !FreeImage_FIFSupportsWriting(format)) {
		return {};
	}

	return widen(FreeImage_GetFormatFromFIF(format));
}
// -----------------------------------------------------------------------------
