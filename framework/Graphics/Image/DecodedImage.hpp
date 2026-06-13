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

#ifndef ANKA_GRAPHICS_IMAGE_DECODEDIMAGE_HPP
#define ANKA_GRAPHICS_IMAGE_DECODEDIMAGE_HPP

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace anka::Graphics::Image
{
	// A decoded image sitting in system memory, in exactly the layout the bitmap
	// factory's createFromMemory expects: 32-bit premultiplied BGRA, top-down,
	// described by (width, height, stride). This is the single currency every
	// decoder produces, so callers never learn or branch on which codec ran.
	//
	// Value type with ownership of its pixels (std::vector): copyable and movable
	// by default, no manual resource management. An empty() image means "decoding
	// failed" — the natural, allocation-free way to signal a miss across the chain.
	class DecodedImage
	{
		public:

			DecodedImage() = default;

			DecodedImage(std::vector<std::byte> pixels, std::uint32_t width, std::uint32_t height, std::uint32_t stride) :
				m_pixels {std::move(pixels)},
				m_width  {width},
				m_height {height},
				m_stride {stride}
			{}

			bool empty(void) const
			{
				return m_pixels.empty();
			}

			std::uint32_t width(void) const
			{
				return m_width;
			}

			std::uint32_t height(void) const
			{
				return m_height;
			}

			std::uint32_t stride(void) const
			{
				return m_stride;
			}

			const void* pixels(void) const
			{
				return m_pixels.data();
			}

		private:

			std::vector<std::byte> m_pixels;
			std::uint32_t          m_width  {0};
			std::uint32_t          m_height {0};
			std::uint32_t          m_stride {0};
	};
}

#endif // ANKA_GRAPHICS_IMAGE_DECODEDIMAGE_HPP
