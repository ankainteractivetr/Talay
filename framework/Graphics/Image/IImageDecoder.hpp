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

#ifndef ANKA_GRAPHICS_IMAGE_IIMAGEDECODER_HPP
#define ANKA_GRAPHICS_IMAGE_IIMAGEDECODER_HPP

#include "ImageSequence.hpp"

#include <string>

namespace anka::Graphics::Image
{
	// Strategy: one image-decoding backend (WIC, FreeImage, ...). Each attempts to
	// decode a file into an ImageSequence and reports whether it could, so the reader
	// can walk an ordered chain and stop at the first backend that handles the file.
	//
	// The currency is ImageSequence, not a single frame, so a backend that supports
	// animation (a GIF/APNG via WIC) fills every frame with its delay, while a
	// still-only backend simply emits a one-frame sequence — callers stay uniform.
	//
	// Deliberately *not* an IObject: a decoder has no identity, no name, no id — it
	// is a pure algorithm. Keeping the interface minimal (one method) is the whole
	// point, so new formats arrive as new strategies, never as edits to the reader.
	class IImageDecoder
	{
		public:

			virtual ~IImageDecoder() noexcept = default;

			// Returns true and fills 'sequence' iff this backend decoded 'path'.
			virtual bool decode(const std::wstring& path, ImageSequence& sequence) const = 0;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IIMAGEDECODER_HPP
