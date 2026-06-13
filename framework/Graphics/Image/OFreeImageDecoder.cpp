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
#include "OFreeImageDecoder.hpp"

#include "FreeImage.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

using namespace anka::Graphics::Image;


// FreeImage plumbing, file-local so FreeImage.h never leaks beyond this unit.
namespace
{
	// RAII over an FIBITMAP* (Effective C++ Item 13): a FreeImage handle frees
	// itself on scope exit, so every early return below stays leak-free.
	struct DibDeleter
	{
		void operator()(FIBITMAP* dib) const noexcept
		{
			FreeImage_Unload(dib);
		}
	};

	using ScopedDib = std::unique_ptr<FIBITMAP, DibDeleter>;

	// Identify by file content first (magic bytes), then by extension as a hint.
	FREE_IMAGE_FORMAT detectFormat(const std::wstring& path)
	{
		const FREE_IMAGE_FORMAT byContent = FreeImage_GetFileTypeU(path.c_str(), 0);
		return byContent != FIF_UNKNOWN ? byContent : FreeImage_GetFIFFromFilenameU(path.c_str());
	}

	// Load and normalise to 32-bit. The raw load is released as we return; the
	// 32-bit copy lives on (ConvertTo32Bits always yields a fresh bitmap).
	ScopedDib load32Bit(const std::wstring& path)
	{
		const FREE_IMAGE_FORMAT format = detectFormat(path);
		if (format == FIF_UNKNOWN) {
			return ScopedDib{ nullptr };
		}

		const ScopedDib raw{ FreeImage_LoadU(format, path.c_str(), 0) };
		return raw ? ScopedDib{ FreeImage_ConvertTo32Bits(raw.get()) } : ScopedDib{ nullptr };
	}

	// FreeImage stores scanlines bottom-up with straight alpha; Direct2D wants
	// top-down premultiplied BGRA — so premultiply then flip before copying.
	DecodedImage toImage(FIBITMAP* dib)
	{
		FreeImage_PreMultiplyWithAlpha(dib);
		FreeImage_FlipVertical(dib);

		const std::uint32_t width  = FreeImage_GetWidth(dib);
		const std::uint32_t height = FreeImage_GetHeight(dib);
		const std::uint32_t stride = FreeImage_GetPitch(dib);
		const BYTE*         bits   = FreeImage_GetBits(dib);
		if (!bits || width == 0 || height == 0) {
			return {};
		}

		const auto* first = reinterpret_cast<const std::byte*>(bits);
		return DecodedImage{ { first, first + static_cast<std::size_t>(stride) * height }, width, height, stride };
	}
}


// -----------------------------------------------------------------------------
// OFreeImageDecoder Implementation
// -----------------------------------------------------------------------------
bool OFreeImageDecoder::decode(const std::wstring& path, ImageSequence& sequence) const
{
	const ScopedDib dib = load32Bit(path);
	if (!dib) {
		return false;
	}

	DecodedImage image = toImage(dib.get());
	if (image.empty()) {
		return false;
	}

	sequence.addFrame({ std::move(image), std::chrono::milliseconds {0} });
	return true;
}
// -----------------------------------------------------------------------------
