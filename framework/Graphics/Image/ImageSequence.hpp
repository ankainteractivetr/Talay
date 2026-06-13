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

#ifndef ANKA_GRAPHICS_IMAGE_IMAGESEQUENCE_HPP
#define ANKA_GRAPHICS_IMAGE_IMAGESEQUENCE_HPP

#include "ImageFrame.hpp"

#include <cstdint>
#include <utility>
#include <vector>

namespace anka::Graphics::Image
{
	// The single currency every decoder now produces: an ordered run of frames plus
	// how often it should loop. A still image and an animation are the *same* type —
	// a still is just the one-frame case (isAnimated() == false) — so callers never
	// branch on "is this a GIF"; they ask isAnimated() and treat both uniformly.
	//
	// loopCount() == 0 means "loop forever" (the GIF/APNG convention); any other
	// value is the exact number of plays. empty() means decoding failed, mirroring
	// DecodedImage so a miss stays allocation-free across the decoder chain.
	class ImageSequence
	{
		public:

			static constexpr std::uint32_t k_loopForever = 0;

			ImageSequence() = default;

			void addFrame(ImageFrame frame)
			{
				m_frames.push_back(std::move(frame));
			}

			void setLoopCount(std::uint32_t loopCount)
			{
				m_loopCount = loopCount;
			}

			bool empty(void) const
			{
				return m_frames.empty();
			}

			std::size_t frameCount(void) const
			{
				return m_frames.size();
			}

			bool isAnimated(void) const
			{
				return m_frames.size() > 1;
			}

			const ImageFrame& frame(std::size_t index) const
			{
				return m_frames.at(index);
			}

			std::uint32_t loopCount(void) const
			{
				return m_loopCount;
			}

		private:

			std::vector<ImageFrame> m_frames;
			std::uint32_t           m_loopCount {k_loopForever};
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IMAGESEQUENCE_HPP
