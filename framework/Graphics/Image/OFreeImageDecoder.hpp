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

#ifndef ANKA_GRAPHICS_IMAGE_OFREEIMAGEDECODER_HPP
#define ANKA_GRAPHICS_IMAGE_OFREEIMAGEDECODER_HPP

#include "IImageDecoder.hpp"

namespace anka::Graphics::Image
{
	// Fallback decoder: FreeImage. It is reached only when WIC declines a file, and
	// it fills the gaps WIC has no OS codec for (PSD, many RAW variants, JPEG-2000,
	// OpenEXR/HDR, TGA, PNM, ...). Deterministic across machines, at the cost of a
	// CPU pass to convert/premultiply/flip into the top-down BGRA we hand upward.
	//
	// Still-only by design: it emits a one-frame ImageSequence. Animation is WIC's
	// job (the primary decoder), so the fallback stays simple and stateless — every
	// FreeImage call (and FreeImage.h) lives inside the .cpp.
	class OFreeImageDecoder final : public IImageDecoder
	{
		public:

			OFreeImageDecoder() = default;

			~OFreeImageDecoder() noexcept override = default;

			bool decode(const std::wstring& path, ImageSequence& sequence) const override;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_OFREEIMAGEDECODER_HPP
