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

#ifndef ANKA_GRAPHICS_BITMAP_ISURFACEBITMAP_HPP
#define ANKA_GRAPHICS_BITMAP_ISURFACEBITMAP_HPP

#include <d2d1.h>

#include <chrono>

namespace anka::Graphics::Bitmap
{
	// A drawable image bound to a surface, abstracting over whether it is a single
	// still or a self-playing animation. The viewport holds one of these and never
	// learns which it has: it draws currentFrame(), and — only if isAnimated() —
	// ticks advance() on a timer keyed to currentDelay(), re-blitting when advance()
	// reports the visible frame changed. (State pattern: the playback state lives in
	// the concrete OContinuousBitmap; OStillBitmap is the inert degenerate case.)
	//
	// Deliberately *not* an IObject — it is pure presentation behaviour with no name
	// or id, and many are created and destroyed as images open (ISP, like the
	// decoder strategies). The frames it returns are owned by it and stay valid for
	// its lifetime; they belong to the surface that created them.
	class ISurfaceBitmap
	{
		public:

			virtual ~ISurfaceBitmap() noexcept = default;

			// True for an animation (more than one frame), false for a still.
			virtual bool isAnimated(void) const = 0;

			// The frame to draw right now; never null for a valid bitmap.
			virtual ID2D1Bitmap* currentFrame(void) const = 0;

			// Intrinsic pixel size of the image (every frame shares it).
			virtual D2D1_SIZE_U pixelSize(void) const = 0;

			// How long the current frame stays up before advance() should be called;
			// zero for a still, the cue not to schedule any playback.
			virtual std::chrono::milliseconds currentDelay(void) const = 0;

			// Step to the next frame; returns true iff the visible frame changed and
			// the caller should re-blit. Always false for a still or a finished,
			// non-looping animation resting on its last frame.
			virtual bool advance(void) = 0;

			// Restart playback from the first frame.
			virtual void reset(void) = 0;
	};
}

#endif // ANKA_GRAPHICS_BITMAP_ISURFACEBITMAP_HPP
