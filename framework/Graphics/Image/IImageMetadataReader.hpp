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

#ifndef ANKA_GRAPHICS_IMAGE_IIMAGEMETADATAREADER_HPP
#define ANKA_GRAPHICS_IMAGE_IIMAGEMETADATAREADER_HPP

#include "ImageMetadata.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <string>

namespace anka::Graphics::Image
{
	// Reads an image file's descriptive metadata without decoding its pixels — the
	// cheap counterpart to IImageFileReader. It pulls geometry, depth and DPI from
	// the image codec and size/timestamps from the file system, returning a single
	// ImageMetadata so callers (the "file info" panel, the library) never learn
	// which OS API supplied which field.
	//
	// An invalid() result (ImageMetadata::valid() == false) means the file could
	// not be inspected, kept allocation-free to match the rest of the image chain.
	class IImageMetadataReader : public anka::Core::Object::IObject
	{
		public:

			virtual ~IImageMetadataReader() noexcept override = default;

			virtual ImageMetadata read(const std::wstring& path) const = 0;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IIMAGEMETADATAREADER_HPP
