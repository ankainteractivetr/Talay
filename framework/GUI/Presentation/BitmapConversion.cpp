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
#include "BitmapConversion.hpp"

#include <robuffer.h>
#include <winrt/Windows.Storage.Streams.h>

#include <algorithm>
#include <cstdint>
#include <cstring>

using anka::Graphics::Image::DecodedImage;
using winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap;


namespace
{
	void copyDecodedPixels(const DecodedImage& image, const WriteableBitmap& bitmap)
	{
		const auto buffer = bitmap.PixelBuffer();
		auto access = buffer.as<::Windows::Storage::Streams::IBufferByteAccess>();
		std::uint8_t* bytes {nullptr};
		winrt::check_hresult(access->Buffer(&bytes));

		const std::size_t available = static_cast<std::size_t>(buffer.Capacity());
		const std::size_t produced  = static_cast<std::size_t>(image.stride()) * image.height();
		std::memcpy(bytes, image.pixels(), (std::min)(available, produced));
		bitmap.Invalidate();
	}
}


// -----------------------------------------------------------------------------
// BitmapConversion Implementation
// -----------------------------------------------------------------------------
WriteableBitmap anka::GUI::Presentation::BitmapConversion::toWriteableBitmap(const DecodedImage& image)
{
	if (image.empty()) {
		return nullptr;
	}

	WriteableBitmap bitmap(static_cast<std::int32_t>(image.width()),
	                       static_cast<std::int32_t>(image.height()));
	copyDecodedPixels(image, bitmap);
	return bitmap;
}
// -----------------------------------------------------------------------------
