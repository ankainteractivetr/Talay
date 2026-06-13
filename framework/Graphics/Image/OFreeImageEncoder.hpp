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

#ifndef ANKA_GRAPHICS_IMAGE_OFREEIMAGEENCODER_HPP
#define ANKA_GRAPHICS_IMAGE_OFREEIMAGEENCODER_HPP

#include "IImageEncoder.hpp"

namespace anka::Graphics::Image
{
	// FreeImage-backed encoder: the write-side counterpart of OFreeImageDecoder. It
	// takes the framework currency — a 32-bit premultiplied BGRA, top-down
	// DecodedImage — undoes the premultiplication, flips it back to FreeImage's
	// bottom-up convention, and saves it in whatever format the destination path's
	// extension names (PNG, JPG, BMP, GIF, TIFF, TGA, WEBP, ...).
	//
	// Formats that cannot carry an alpha channel (JPEG, plain BMP, ...) are written
	// as 24-bit; FreeImage reports this via FIFSupportsExportBPP, so the codec list
	// stays data-driven and this class never special-cases a format. Every FreeImage
	// call (and FreeImage.h) lives inside the .cpp — stateless and header-clean.
	class OFreeImageEncoder final : public IImageEncoder
	{
		public:

			OFreeImageEncoder() = default;

			~OFreeImageEncoder() noexcept override = default;

			bool encode(const DecodedImage& image, const std::wstring& path) const override;

			std::wstring encodableFormatKey(const std::wstring& extension) const override;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_OFREEIMAGEENCODER_HPP
