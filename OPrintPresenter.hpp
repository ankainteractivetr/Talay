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

#ifndef TALAY_PRINT_OPRINTPRESENTER_HPP
#define TALAY_PRINT_OPRINTPRESENTER_HPP

#include "IPrintPresenter.hpp"
#include "framework/Graphics/Image/IImageFileReader.hpp"
#include "framework/Graphics/Print/IImagePrinter.hpp"

#include <memory>

namespace talay
{
	namespace Print
	{
		/*
		*
		* Concrete Print Presenter Class
		*
		* Dependency-injected with the codec-agnostic file reader and the
		* system print facade; owns nothing but the wiring between them.
		*
		*/
		class OPrintPresenter final : public IPrintPresenter
		{
			public:

				OPrintPresenter(anka::Graphics::Image::IImageFileReader& reader,
				                anka::Graphics::Print::IImagePrinter& printer);
				OPrintPresenter(const OPrintPresenter& object) = delete;
				OPrintPresenter(OPrintPresenter&& object) noexcept = delete;

				~OPrintPresenter() noexcept override;

				OPrintPresenter& operator=(const OPrintPresenter& object) = delete;
				OPrintPresenter& operator=(OPrintPresenter&& object) noexcept = delete;

				//
				// IObject overrides
				//

				const std::wstring& getName(void) const override;
				void                setName(const std::wstring& name) override;

				std::uint64_t getId(void) const override;

				//
				// IPrintPresenter overrides
				//

				anka::Graphics::Print::PrintOutcome printFile(const std::wstring& path, HWND owner) const override;

			private:

				struct Impl;
				std::unique_ptr<Impl> m_pImpl;
		};
	}
}

#endif // TALAY_PRINT_OPRINTPRESENTER_HPP
