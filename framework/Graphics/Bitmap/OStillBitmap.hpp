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

#ifndef ANKA_GRAPHICS_BITMAP_OSTILLBITMAP_HPP
#define ANKA_GRAPHICS_BITMAP_OSTILLBITMAP_HPP

#include "ISurfaceBitmap.hpp"

#include <d2d1.h>

#include <chrono>
#include <memory>

namespace anka::Graphics::Bitmap
{
	/*
	*
	* Concrete still (single-frame) surface bitmap
	*
	* The inert case of ISurfaceBitmap: one frame that never advances. isAnimated()
	* is false and currentDelay() is zero, so the viewport draws it once and starts
	* no playback timer.
	*
	*/
	class OStillBitmap final : public ISurfaceBitmap
	{
		public:

			// Borrows a reference to 'frame' (AddRef'd internally) for its lifetime.
			OStillBitmap(ID2D1Bitmap* frame, const D2D1_SIZE_U& size);

			OStillBitmap(const OStillBitmap& bitmap) = delete;
			OStillBitmap(OStillBitmap&& bitmap) noexcept = delete;

			~OStillBitmap() noexcept override;

			OStillBitmap& operator=(const OStillBitmap& bitmap) = delete;
			OStillBitmap& operator=(OStillBitmap&& bitmap) noexcept = delete;

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

#endif // ANKA_GRAPHICS_BITMAP_OSTILLBITMAP_HPP
