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

#ifndef ANKA_CORE_IO_IFILEPICKER_HPP
#define ANKA_CORE_IO_IFILEPICKER_HPP

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.Storage.Pickers.h>

#include "framework/Core/Object/IObject.hpp"
#include "framework/System/Windows/windows.hpp"

#include <string>
#include <vector>

namespace anka
{
	namespace Core
	{
		namespace IO
		{
			// One selectable type for a multi-format save dialog: the label the
			// dialog's type dropdown shows and the dotted extension it enforces.
			struct SaveFileType
			{
				std::wstring typeName;    // e.g. L"PNG"
				std::wstring extension;   // dotted, e.g. L".png"
			};

			/*
			* Abstract File Picker Interface
			*
			* A File Picker picks a file to be processed
			*
			*/

			class IFilePicker : public anka::Core::Object::IObject
			{
				public:

					virtual ~IFilePicker() noexcept = default;

					virtual void setOwner(HWND owner) = 0;

					virtual winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> pickFile(const std::vector<std::wstring>& formats) = 0;

					// Ask where to save a file of one type: 'typeName' is the human
					// label shown for 'extension' (e.g. L"SQL dump" / L".sql").
					// Returns the chosen full path, empty if the user cancelled.
					virtual winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> pickSaveFile(const std::wstring& typeName,
					                                                                                 const std::wstring& extension,
					                                                                                 const std::wstring& suggestedName) = 0;

					// As pickSaveFile, but offering several types at once — the
					// dialog's own type dropdown picks the format, so directory,
					// name AND format come back as one chosen path. Empty when
					// the user cancelled, or when 'types' offered nothing.
					virtual winrt::Windows::Foundation::IAsyncOperation<winrt::hstring> pickSaveFileAs(const std::vector<SaveFileType>& types,
					                                                                                   const std::wstring& suggestedName) = 0;

					virtual winrt::Windows::Storage::Pickers::FileOpenPicker makePicker(const std::vector<std::wstring>& formats) = 0;
			};
		}
	}
}

#endif // ANKA_CORE_IO_IFILEPICKER_HPP