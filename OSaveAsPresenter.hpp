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

#ifndef TALAY_SAVE_OSAVEASPRESENTER_HPP
#define TALAY_SAVE_OSAVEASPRESENTER_HPP

#include "ISaveAsPresenter.hpp"
#include "framework/Graphics/Conversion/IImageConverter.hpp"

#include <memory>

namespace talay::Save
{
	/*
	*
	* Concrete Save-As Presenter Class
	*
	* Dependency-injected with the codec-agnostic converter facade; owns
	* nothing but the wiring between the picked destination path and the
	* converter's request vocabulary (directory + name + extension).
	*
	*/
	class OSaveAsPresenter final : public ISaveAsPresenter
	{
		public:

			explicit OSaveAsPresenter(anka::Graphics::Conversion::IImageConverter& converter);
			OSaveAsPresenter(const OSaveAsPresenter& object) = delete;
			OSaveAsPresenter(OSaveAsPresenter&& object) noexcept = delete;

			~OSaveAsPresenter() noexcept override;

			OSaveAsPresenter& operator=(const OSaveAsPresenter& object) = delete;
			OSaveAsPresenter& operator=(OSaveAsPresenter&& object) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// ISaveAsPresenter overrides
			//

			std::vector<anka::Graphics::Conversion::WritableFormat> formats(void) const override;

			bool saveAs(const std::wstring& sourcePath,
			            const std::wstring& destinationPath) const override;

			bool saveAsWithEffect(const std::wstring& sourcePath,
			                      const std::wstring& destinationPath,
			                      const anka::Graphics::Effect::IPixelEffect& effect) const override;

		private:

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // TALAY_SAVE_OSAVEASPRESENTER_HPP
