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

#ifndef ANKA_GRAPHICS_IMAGE_OWICIMAGEDECODER_HPP
#define ANKA_GRAPHICS_IMAGE_OWICIMAGEDECODER_HPP

#include "IImageDecoder.hpp"

namespace anka::Graphics::Image
{
	// Primary decoder: the Windows Imaging Component. It ships with the OS (no DLL
	// to deploy, kept patched by Windows Update), decodes the common formats plus
	// HEIC/WebP/AVIF when their OS codec extensions are present, and converts
	// straight to premultiplied BGRA — the layout Direct2D wants. Tried first.
	//
	// Handles animation too: a multi-frame container (GIF, APNG, animated WebP) is
	// composited frame-by-frame onto a full-canvas accumulator — honouring each
	// frame's sub-rectangle, disposal method and delay — into a timed ImageSequence.
	// A single-frame file collapses to a one-frame sequence.
	//
	// Stateless: all WIC plumbing (and <wincodec.h>) is sealed inside the .cpp, so
	// nothing here leaks COM into the rest of the build.
	class OWicImageDecoder final : public IImageDecoder
	{
		public:

			OWicImageDecoder() = default;

			~OWicImageDecoder() noexcept override = default;

			bool decode(const std::wstring& path, ImageSequence& sequence) const override;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_OWICIMAGEDECODER_HPP
