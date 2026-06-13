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

#ifndef ANKA_GRAPHICS_PRINT_PAGEFITTING_HPP
#define ANKA_GRAPHICS_PRINT_PAGEFITTING_HPP

#include <algorithm>
#include <cstdint>

namespace anka::Graphics::Print
{
	// The printable page as the device reports it: its size in device pixels
	// plus the pixel density per axis. Density rides along because printer
	// pixels are not always square — fitting has to reason in physical units
	// or a 300x600 dpi device would stretch every photo.
	struct PageMetrics
	{
		std::int32_t width  {0};    // printable width,  device pixels
		std::int32_t height {0};    // printable height, device pixels
		std::int32_t dpiX   {96};   // pixels per inch, horizontal
		std::int32_t dpiY   {96};   // pixels per inch, vertical
	};

	// Where the image lands on the page, in device pixels.
	struct PageFrame
	{
		std::int32_t x      {0};
		std::int32_t y      {0};
		std::int32_t width  {0};
		std::int32_t height {0};
	};

	// The largest frame that shows the whole image on the page, centred, with
	// its physical aspect ratio intact: scale in inches first, then convert
	// back per axis. A pure function of its inputs (no device context), so the
	// one piece of print geometry stays independently testable.
	inline PageFrame fitCentered(std::uint32_t imageWidth, std::uint32_t imageHeight, const PageMetrics& page)
	{
		if (imageWidth == 0 || imageHeight == 0 || page.dpiX <= 0 || page.dpiY <= 0)
			return {};

		const double physicalWidth  {page.width  / static_cast<double>(page.dpiX)};
		const double physicalHeight {page.height / static_cast<double>(page.dpiY)};
		const double inchesPerPixel {(std::min)(physicalWidth / imageWidth, physicalHeight / imageHeight)};

		const auto width  {static_cast<std::int32_t>(imageWidth  * inchesPerPixel * page.dpiX)};
		const auto height {static_cast<std::int32_t>(imageHeight * inchesPerPixel * page.dpiY)};
		return {.x = (page.width - width) / 2, .y = (page.height - height) / 2, .width = width, .height = height};
	}
}

#endif // ANKA_GRAPHICS_PRINT_PAGEFITTING_HPP
