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

#ifndef ANKA_GRAPHICS_IMAGE_IIMAGEFILEREADER_HPP
#define ANKA_GRAPHICS_IMAGE_IIMAGEFILEREADER_HPP

#include "DecodedImage.hpp"
#include "ImageSequence.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <string>

namespace anka::Graphics::Image
{
	// Reads an image file, hiding which codec did the work. It tries the Windows
	// Imaging Component first (native, kept patched, broad OS codec coverage) and
	// falls back to FreeImage for formats WIC does not handle.
	//
	// readSequence is the primary operation: it yields every frame with its timing,
	// so an animated GIF plays and a still image is just a one-frame sequence. read
	// is the still-only convenience (the first frame) for callers — thumbnails, the
	// library — that never animate. The decoder-agnostic result keeps the caller,
	// and the bitmap factory it feeds, free of any codec specifics.
	class IImageFileReader : public anka::Core::Object::IObject
	{
		public:

			virtual ~IImageFileReader() noexcept override = default;

			// Empty ImageSequence when no decoder in the chain could read the file.
			virtual ImageSequence readSequence(const std::wstring& path) const = 0;

			// First frame only; empty DecodedImage on failure.
			virtual DecodedImage read(const std::wstring& path) const = 0;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IIMAGEFILEREADER_HPP
