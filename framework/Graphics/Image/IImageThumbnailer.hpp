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

#ifndef ANKA_GRAPHICS_IMAGE_IIMAGETHUMBNAILER_HPP
#define ANKA_GRAPHICS_IMAGE_IIMAGETHUMBNAILER_HPP

#include "DecodedImage.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <cstdint>
#include <string>

namespace anka::Graphics::Image
{
	// Produces a small, display-ready still for the folder reel — the gallery
	// counterpart of IImageFileReader. Where the reader returns the image at full
	// resolution to be viewed, the thumbnailer returns it shrunk so a whole folder
	// can be shown cheaply, scaling at decode time rather than after.
	//
	// The result is the same framework currency every decoder speaks (32-bit
	// premultiplied BGRA, top-down), so the caller copies it into a bitmap exactly
	// as it would a full frame. An empty DecodedImage means the file could not be
	// read at all — the throw-free "show a placeholder" signal.
	class IImageThumbnailer : public anka::Core::Object::IObject
	{
		public:

			virtual ~IImageThumbnailer() noexcept override = default;

			// A still whose longest side is at most 'maxEdge' pixels (smaller images
			// are returned unscaled); empty DecodedImage when the file is unreadable.
			virtual DecodedImage thumbnail(const std::wstring& path, std::uint32_t maxEdge) const = 0;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IIMAGETHUMBNAILER_HPP
