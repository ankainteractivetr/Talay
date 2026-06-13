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

#ifndef ANKA_GRAPHICS_IMAGE_IIMAGEENCODER_HPP
#define ANKA_GRAPHICS_IMAGE_IIMAGEENCODER_HPP

#include "DecodedImage.hpp"

#include <string>

namespace anka::Graphics::Image
{
	// Strategy: one image-encoding backend (FreeImage, WIC, ...). It is the exact
	// mirror of IImageDecoder — where the decoder turns a file into a DecodedImage,
	// the encoder writes a DecodedImage back out to a file. The target format is
	// taken from the destination path's extension, so the same DecodedImage can be
	// saved as PNG, JPG, BMP, ... without the caller knowing which codec ran.
	//
	// Deliberately *not* an IObject: an encoder has no identity, no name, no id —
	// it is a pure algorithm. Keeping the interface minimal (one method) is the
	// whole point, so new output formats arrive as new strategies, never as edits
	// to the converter that drives the chain (Open/Closed).
	class IImageEncoder
	{
		public:

			virtual ~IImageEncoder() noexcept = default;

			// Returns true iff this backend wrote 'image' to 'path' (format inferred
			// from the path's extension).
			virtual bool encode(const DecodedImage& image, const std::wstring& path) const = 0;

			// A canonical key naming the codec this backend would use to write the
			// given extension (e.g. ".JPG" and ".JPEG" both yield "JPEG"), or an empty
			// string when the backend cannot write that format. The converter uses it
			// to offer only writable formats and to collapse the aliases of one codec
			// into a single choice — without the caller learning any codec specifics.
			virtual std::wstring encodableFormatKey(const std::wstring& extension) const = 0;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IIMAGEENCODER_HPP
