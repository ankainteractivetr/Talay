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

#ifndef ANKA_GRAPHICS_IMAGE_OIMAGEMETADATAREADER_HPP
#define ANKA_GRAPHICS_IMAGE_OIMAGEMETADATAREADER_HPP

#include "IImageMetadataReader.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace anka::Graphics::Image
{
	/*
	*
	* Concrete Image Metadata Reader Class
	*
	* Gathers image geometry/depth/DPI through the Windows Imaging Component and
	* on-disk size/timestamps through the Win32 file API. All platform plumbing is
	* sealed behind the pimpl, so no COM or <windows.h> leaks past this type.
	*
	*/
	class OImageMetadataReader final : public IImageMetadataReader
	{
		public:

			// Singleton
			static OImageMetadataReader& getInstance();

			OImageMetadataReader(const OImageMetadataReader& reader) = delete;
			OImageMetadataReader(OImageMetadataReader&& reader) noexcept = delete;

			~OImageMetadataReader() noexcept override;

			OImageMetadataReader& operator=(const OImageMetadataReader& reader) = delete;
			OImageMetadataReader& operator=(OImageMetadataReader&& reader) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// IImageMetadataReader overrides
			//

			ImageMetadata read(const std::wstring& path) const override;

		private:

			// Singleton COTOR
			OImageMetadataReader();

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_GRAPHICS_IMAGE_OIMAGEMETADATAREADER_HPP
