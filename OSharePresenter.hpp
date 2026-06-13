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

#ifndef TALAY_SHARE_OSHAREPRESENTER_HPP
#define TALAY_SHARE_OSHAREPRESENTER_HPP

#include "ISharePresenter.hpp"
#include "framework/Graphics/Image/IImageFileReader.hpp"
#include "framework/System/Clipboard/IImageClipboard.hpp"

#include <memory>

namespace talay
{
	namespace Share
	{
		/*
		*
		* Concrete Share Presenter Class
		*
		* Dependency-injected with the codec-agnostic file reader and the
		* system clipboard facade; owns nothing but the wiring between them.
		*
		*/
		class OSharePresenter final : public ISharePresenter
		{
			public:

				OSharePresenter(anka::Graphics::Image::IImageFileReader& reader,
				                anka::System::Clipboard::IImageClipboard& clipboard);
				OSharePresenter(const OSharePresenter& object) = delete;
				OSharePresenter(OSharePresenter&& object) noexcept = delete;

				~OSharePresenter() noexcept override;

				OSharePresenter& operator=(const OSharePresenter& object) = delete;
				OSharePresenter& operator=(OSharePresenter&& object) noexcept = delete;

				//
				// IObject overrides
				//

				const std::wstring& getName(void) const override;
				void                setName(const std::wstring& name) override;

				std::uint64_t getId(void) const override;

				//
				// ISharePresenter overrides
				//

				bool shareFile(const std::wstring& path, HWND owner) const override;

				bool shareFileWithEffect(const std::wstring& path, HWND owner,
				                         const anka::Graphics::Effect::IPixelEffect& effect) const override;

			private:

				struct Impl;
				std::unique_ptr<Impl> m_pImpl;
		};
	}
}

#endif // TALAY_SHARE_OSHAREPRESENTER_HPP
