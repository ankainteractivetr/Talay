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

#ifndef ANKA_GRAPHICS_IMAGE_OWICIMAGETHUMBNAILER_HPP
#define ANKA_GRAPHICS_IMAGE_OWICIMAGETHUMBNAILER_HPP

#include "IImageThumbnailer.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace anka::Graphics::Image
{
	// WIC-backed thumbnailer. It decodes only the first frame and asks the Windows
	// Imaging Component to scale it down with IWICBitmapScaler, so a large photo is
	// never fully expanded in memory just to be shown the size of a postage stamp.
	//
	// WIC covers the common formats natively; for the rest (the RAW family and other
	// FreeImage-only codecs) it falls back to the full IImageFileReader chain, so the
	// reel can still show every file the viewer can open. Every WIC/COM call lives in
	// the .cpp — the header stays clean.
	class OWicImageThumbnailer final : public IImageThumbnailer
	{
		public:

			// Singleton
			static OWicImageThumbnailer& getInstance();

			OWicImageThumbnailer(const OWicImageThumbnailer& thumbnailer) = delete;
			OWicImageThumbnailer(OWicImageThumbnailer&& thumbnailer) noexcept = delete;

			~OWicImageThumbnailer() noexcept override;

			OWicImageThumbnailer& operator=(const OWicImageThumbnailer& thumbnailer) = delete;
			OWicImageThumbnailer& operator=(OWicImageThumbnailer&& thumbnailer) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// IImageThumbnailer overrides
			//

			DecodedImage thumbnail(const std::wstring& path, std::uint32_t maxEdge) const override;

		private:

			// Singleton COTOR
			OWicImageThumbnailer();

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_OWICIMAGETHUMBNAILER_HPP
