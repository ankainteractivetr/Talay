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

#ifndef ANKA_GRAPHICS_BITMAP_OCONTINUOUSBITMAP_HPP
#define ANKA_GRAPHICS_BITMAP_OCONTINUOUSBITMAP_HPP

#include "ISurfaceBitmap.hpp"

#include <d2d1.h>

#include <chrono>
#include <cstdint>
#include <memory>

namespace anka::Graphics::Bitmap
{
	/*
	*
	* Concrete continuous (animated) surface bitmap
	*
	* Holds the whole frame run and owns playback: advance() walks to the next frame
	* and loops up to loopCount times (zero = forever), reporting when the visible
	* frame changed so the viewport re-blits. Built one frame at a time by
	* OSurfaceBitmapFactory; consumers only see it through ISurfaceBitmap.
	*
	*/
	class OContinuousBitmap final : public ISurfaceBitmap
	{
		public:

			OContinuousBitmap(const D2D1_SIZE_U& size, std::uint32_t loopCount);

			OContinuousBitmap(const OContinuousBitmap& bitmap) = delete;
			OContinuousBitmap(OContinuousBitmap&& bitmap) noexcept = delete;

			~OContinuousBitmap() noexcept override;

			OContinuousBitmap& operator=(const OContinuousBitmap& bitmap) = delete;
			OContinuousBitmap& operator=(OContinuousBitmap&& bitmap) noexcept = delete;

			// Append a frame (AddRef'd internally). Called by the factory while it
			// assembles the bitmap; the order of calls is the playback order.
			void addFrame(ID2D1Bitmap* frame, std::chrono::milliseconds delay);

			//
			// ISurfaceBitmap overrides
			//

			bool                      isAnimated(void) const override;
			ID2D1Bitmap*              currentFrame(void) const override;
			D2D1_SIZE_U               pixelSize(void) const override;
			std::chrono::milliseconds currentDelay(void) const override;
			bool                      advance(void) override;
			void                      reset(void) override;

		private:

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_GRAPHICS_BITMAP_OCONTINUOUSBITMAP_HPP
