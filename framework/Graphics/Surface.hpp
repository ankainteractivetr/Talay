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

#ifndef ANKA_GRAPHICS_SURFACE_HPP
#define ANKA_GRAPHICS_SURFACE_HPP

#include <cstdint>

namespace anka::Graphics
{
	// Opaque handle to a render surface owned by the graphics resource keeper.
	// A surface is one offscreen Direct2D target: the viewport, or a single reel
	// or library thumbnail. Handles are minted on demand (createSurface) and
	// released when their owner goes away (destroySurface), so a virtualized list
	// can spin a surface up and down per realized item.
	//
	// Ids are monotonic and never reused; a stale handle can never alias a freshly
	// created surface. k_invalidSurface (0) is the null handle.
	using SurfaceId = std::uint64_t;

	inline constexpr SurfaceId k_invalidSurface = 0;
}

#endif // ANKA_GRAPHICS_SURFACE_HPP
