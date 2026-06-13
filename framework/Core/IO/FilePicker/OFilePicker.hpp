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

#ifndef ANKA_CORE_IO_OFILEPICKER_HPP
#define ANKA_CORE_IO_OFILEPICKER_HPP

#include "IFilePicker.hpp"

#include <memory>

namespace anka
{
	namespace Core
	{
		namespace IO
		{
			/*
			*
			* Concerete File Picker Class
			*
			*/
			class OFilePicker final : public IFilePicker
			{
				public:

				   // Singleton
					static OFilePicker& getInstance();

					OFilePicker(const OFilePicker& picker) = delete;
					OFilePicker(OFilePicker&& picker) noexcept = delete;

					~OFilePicker() noexcept override;

					OFilePicker& operator=(const OFilePicker& picker) = delete;
					OFilePicker& operator=(OFilePicker&& picker) noexcept = delete;

					

					//
					// IObject overrides
					//

					const std::wstring& getName(void) const override;
					void                setName(const std::wstring& name) override;

					std::uint64_t getId(void) const override;

					//
					// IFilePicker overrides
					//

					void setOwner(HWND owner) override;

					virtual winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> pickFile(const std::vector<std::wstring>& formats) override;

					virtual winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> pickSaveFile(const std::wstring& typeName,
					                                                                                 const std::wstring& extension,
					                                                                                 const std::wstring& suggestedName) override;

					virtual winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> pickSaveFileAs(const std::vector<SaveFileType>& types,
					                                                                                   const std::wstring& suggestedName) override;

				private:

					//Singleton COTOR
					OFilePicker();

					//
					// IFilePicker overrides
					//			

					virtual winrt::Windows::Storage::Pickers::FileOpenPicker makePicker(const std::vector<std::wstring>& formats) override;

					struct Impl;
					std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}

#endif // ANKA_CORE_IO_OFILEPICKER_HPP