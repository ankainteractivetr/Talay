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

#ifndef ANKA_GRAPHICS_IMAGE_IMAGEFRAME_HPP
#define ANKA_GRAPHICS_IMAGE_IMAGEFRAME_HPP

#include "DecodedImage.hpp"

#include <chrono>
#include <utility>

namespace anka::Graphics::Image
{
	// One frame of an image plus how long it stays on screen before the next one.
	// A still image is simply a single frame whose delay is irrelevant (zero); an
	// animation is an ordered run of these. The delay is what GIF/APNG/WebP encode
	// per frame, so timing rides alongside the pixels instead of in a side table.
	//
	// Value type that owns its pixels through DecodedImage: copyable and movable by
	// default, no manual resource management.
	class ImageFrame
	{
		public:

			ImageFrame() = default;

			ImageFrame(DecodedImage image, std::chrono::milliseconds delay) :
				m_image {std::move(image)},
				m_delay {delay}
			{}

			bool empty(void) const
			{
				return m_image.empty();
			}

			const DecodedImage& image(void) const
			{
				return m_image;
			}

			std::chrono::milliseconds delay(void) const
			{
				return m_delay;
			}

		private:

			DecodedImage              m_image;
			std::chrono::milliseconds m_delay {0};
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IMAGEFRAME_HPP
