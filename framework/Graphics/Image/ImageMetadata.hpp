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

#ifndef ANKA_GRAPHICS_IMAGE_IMAGEMETADATA_HPP
#define ANKA_GRAPHICS_IMAGE_IMAGEMETADATA_HPP

#include <cstdint>
#include <string>

namespace anka::Graphics::Image
{
	// Everything the "file info" surface needs about a single image, gathered into
	// one decoder-agnostic value: pixel geometry from the image codec (WIC) and
	// on-disk facts (size, timestamps) from the file system. The fields stay raw —
	// numbers as numbers, dates as already-localised strings (formatting a FILETIME
	// is a file-system concern, so the reader does it once) — leaving every
	// presentation choice (KB vs MB, "W × H", aspect reduction) to the view.
	// The last-write stamp is additionally kept as raw UTC seconds since the Unix
	// epoch (modifiedUnixUtc) for consumers that compare dates rather than show
	// them — the library's date filter sorts and ranges on numbers, not strings.
	//
	// Plain value type that owns its data (std::wstring / std::uint*): copyable and
	// movable by default, no manual resource management. valid() == false means the
	// reader could not open the file — the allocation-free way to signal a miss,
	// mirroring DecodedImage::empty().
	class ImageMetadata
	{
		public:

			std::wstring  fileName;
			std::wstring  filePath;
			std::wstring  format;             // container, e.g. L"PNG"
			std::wstring  pixelFormat;        // human channels, e.g. L"BGRA"
			std::wstring  colorProfile;       // e.g. L"sRGB" / L"Embedded ICC"
			std::wstring  created;            // localised date-time, file system
			std::wstring  modified;           // localised date-time, file system

			std::uint32_t width        {0};
			std::uint32_t height       {0};
			std::uint32_t bitDepth     {0};   // bits per pixel
			std::uint32_t channelCount {0};
			std::uint32_t frameCount   {1};   // > 1 means animated (e.g. GIF)

			double        dpiX {0.0};
			double        dpiY {0.0};

			std::uint64_t fileSizeBytes   {0};
			std::int64_t  modifiedUnixUtc {0};   // last write, seconds since Unix epoch

			bool valid(void) const
			{
				return width != 0 && height != 0;
			}
	};
}

#endif // ANKA_GRAPHICS_IMAGE_IMAGEMETADATA_HPP
