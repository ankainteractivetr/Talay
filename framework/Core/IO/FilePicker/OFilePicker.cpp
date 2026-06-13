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

#include "pch.h"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "OFilePicker.hpp"

// IInitializeWithWindow is declared in <Shobjidl.h>, but that header also pulls
// in shell interfaces (IShellLibrary, ...) whose IID_PPV_ARGS templates use an
// unqualified IUnknown. With winrt's IUnknown in scope (leaked transitively via
// `using namespace winrt::Windows::Foundation` -> `using namespace anka::Core::IO`)
// that name becomes ambiguous (C2872). We only need this one interface, so we
// declare it here against the global ::IUnknown and avoid the header entirely.
struct __declspec(uuid("3E68D4BD-7135-4D10-8018-9FB6D9F33FA1")) __declspec(novtable)
IInitializeWithWindow : ::IUnknown
{
	virtual HRESULT __stdcall Initialize(HWND hwnd) = 0;
};

using namespace anka::Core::Object;
using namespace anka::Core::IO;
using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Pickers;

// -----------------------------------------------------------------------------
// OFilePicker::Impl Definition
// -----------------------------------------------------------------------------
struct OFilePicker::Impl
{
	ObjectIdentity m_identity {L"FilePickerObject"};
	HWND           m_owner {NULL};

	const std::wstring& getObjectName(void) const
	{
		return m_identity.name();
	}

	void setObjectName(const std::wstring& name)
	{
		m_identity.setName(name);
	}

	std::uint64_t getObjectId(void) const
	{
		return m_identity.id();
	}

	void setOwner(HWND owner)
	{
		m_owner = owner;
	}

	// On desktop the pickers have no implicit window to anchor to, so each must
	// be associated with the owner HWND or its PickAsync call throws. Taking
	// IInspectable lets one anchor serve both the open and the save picker.
	void anchorToOwner(const winrt::Windows::Foundation::IInspectable& picker) const
	{
		check_hresult(picker.as<::IInitializeWithWindow>()->Initialize(m_owner));
	}

	// FileTypeFilter must contain at least one entry, or it throws E_INVALIDARG.
	static void applyFilters(const FileOpenPicker& picker, const std::vector<std::wstring>& formats)
	{
		auto filter = picker.FileTypeFilter();
		if (formats.empty()) {
			filter.Append(L"*");
			return;
		}
		for (const auto& format : formats) {
			filter.Append(hstring {format});
		}
	}

	FileOpenPicker makePicker(const std::vector<std::wstring>& formats)
	{
		FileOpenPicker picker;
		anchorToOwner(picker);
		picker.SuggestedStartLocation(PickerLocationId::Desktop);
		picker.ViewMode(PickerViewMode::Thumbnail);
		applyFilters(picker, formats);
		return picker;
	}

	IAsyncOperation<hstring> pickFile(const std::vector<std::wstring>& formats)
	{
		FileOpenPicker picker {makePicker(formats)};

		StorageFile file {co_await picker.PickSingleFileAsync()};

		// Return the full path, or an empty string if the user cancelled.
		co_return (file != nullptr) ? file.Path() : hstring{};
	}

	// FileTypeChoices must contain at least one labelled type, or the picker
	// throws E_INVALIDARG — hence one mandatory typeName/extension pair.
	FileSavePicker makeSavePicker(const std::wstring& typeName,
	                              const std::wstring& extension,
	                              const std::wstring& suggestedName)
	{
		FileSavePicker picker;
		anchorToOwner(picker);
		picker.SuggestedStartLocation(PickerLocationId::DocumentsLibrary);
		picker.SuggestedFileName(hstring {suggestedName});
		picker.FileTypeChoices().Insert(hstring {typeName},
		                                winrt::single_threaded_vector<hstring>({hstring {extension}}));
		return picker;
	}

	IAsyncOperation<hstring> pickSaveFile(const std::wstring& typeName,
	                                      const std::wstring& extension,
	                                      const std::wstring& suggestedName)
	{
		FileSavePicker picker {makeSavePicker(typeName, extension, suggestedName)};

		StorageFile file {co_await picker.PickSaveFileAsync()};

		// Return the full path, or an empty string if the user cancelled.
		co_return (file != nullptr) ? file.Path() : hstring{};
	}

	// FileTypeChoices preserves insertion order, so the caller's first type is
	// the dialog's default.
	static void applyTypeChoices(const FileSavePicker& picker, const std::vector<SaveFileType>& types)
	{
		for (const auto& type : types) {
			picker.FileTypeChoices().Insert(hstring {type.typeName},
			                                winrt::single_threaded_vector<hstring>({hstring {type.extension}}));
		}
	}

	FileSavePicker makeMultiTypeSavePicker(const std::vector<SaveFileType>& types,
	                                       const std::wstring& suggestedName)
	{
		FileSavePicker picker;
		anchorToOwner(picker);
		picker.SuggestedStartLocation(PickerLocationId::Desktop);
		picker.SuggestedFileName(hstring {suggestedName});
		applyTypeChoices(picker, types);
		return picker;
	}

	IAsyncOperation<hstring> pickSaveFileAs(const std::vector<SaveFileType>& types,
	                                        const std::wstring& suggestedName)
	{
		// No offerable type = nothing the dialog could enforce; treat as cancel
		// (an empty FileTypeChoices would throw E_INVALIDARG).
		if (types.empty())
			co_return hstring {};

		FileSavePicker picker {makeMultiTypeSavePicker(types, suggestedName)};

		StorageFile file {co_await picker.PickSaveFileAsync()};

		co_return (file != nullptr) ? file.Path() : hstring{};
	}
};

// -----------------------------------------------------------------------------
// OFilePicker Implementation
// -----------------------------------------------------------------------------
OFilePicker::OFilePicker() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OFilePicker::~OFilePicker() noexcept = default;
// -----------------------------------------------------------------------------
OFilePicker& OFilePicker::getInstance()
{
	static OFilePicker s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OFilePicker::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OFilePicker::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OFilePicker::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
void OFilePicker::setOwner(HWND owner)
{
	m_pImpl->setOwner(owner);
}
// -----------------------------------------------------------------------------
FileOpenPicker OFilePicker::makePicker(const std::vector<std::wstring>& formats)
{
	return m_pImpl->makePicker(formats);
}
// -----------------------------------------------------------------------------
IAsyncOperation<hstring> OFilePicker::pickFile(const std::vector<std::wstring>& formats)
{
	return m_pImpl->pickFile(formats);
}
// -----------------------------------------------------------------------------
IAsyncOperation<hstring> OFilePicker::pickSaveFile(const std::wstring& typeName,
                                                   const std::wstring& extension,
                                                   const std::wstring& suggestedName)
{
	return m_pImpl->pickSaveFile(typeName, extension, suggestedName);
}
// -----------------------------------------------------------------------------
IAsyncOperation<hstring> OFilePicker::pickSaveFileAs(const std::vector<SaveFileType>& types,
                                                     const std::wstring& suggestedName)
{
	return m_pImpl->pickSaveFileAs(types, suggestedName);
}
// -----------------------------------------------------------------------------
