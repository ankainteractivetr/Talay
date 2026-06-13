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
#include "OFolderImageScanner.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <unordered_set>
#include <vector>

using namespace anka::Core::IO;
using namespace anka::Core::Object;


// Pure path / string arithmetic, file-local so <filesystem> never leaks past this
// unit and the scanner stays a thin orchestrator.
namespace
{
	std::wstring toUpper(std::wstring text)
	{
		std::transform(text.begin(), text.end(), text.begin(),
		               [](wchar_t character) { return static_cast<wchar_t>(std::towupper(character)); });
		return text;
	}

	// The candidate extensions, upper-cased into a set so membership is O(1) and
	// case-insensitive (the file system mixes ".JPG" and ".jpg" freely).
	std::unordered_set<std::wstring> buildExtensionSet(const std::vector<std::wstring>& extensions)
	{
		std::unordered_set<std::wstring> set;
		for (const auto& extension : extensions) {
			set.insert(toUpper(extension));
		}
		return set;
	}

	bool isSupportedImage(const std::filesystem::directory_entry& entry,
	                      const std::unordered_set<std::wstring>& extensions)
	{
		return entry.is_regular_file() && extensions.contains(toUpper(entry.path().extension().wstring()));
	}

	// Collect every supported file in 'folder'; an unreadable folder yields the
	// empty list (error_code overload — no throw on a vanished or denied path).
	std::vector<std::wstring> collect(const std::filesystem::path& folder,
	                                  const std::unordered_set<std::wstring>& extensions)
	{
		std::error_code error;
		std::vector<std::wstring> paths;
		for (const auto& entry : std::filesystem::directory_iterator(folder, error)) {
			if (isSupportedImage(entry, extensions)) {
				paths.push_back(entry.path().wstring());
			}
		}
		return paths;
	}

	bool namePrecedes(const std::wstring& left, const std::wstring& right)
	{
		return toUpper(std::filesystem::path{left}.filename().wstring())
		     < toUpper(std::filesystem::path{right}.filename().wstring());
	}
}


// -----------------------------------------------------------------------------
// OFolderImageScanner::Impl Definition
// -----------------------------------------------------------------------------
struct OFolderImageScanner::Impl
{
	ObjectIdentity m_identity {L"FolderImageScannerObject"};

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

	// Enumerate the reference image's folder, keep the supported files and order
	// them by name so the reel reads the same way the file explorer does.
	std::vector<std::wstring> scan(const std::wstring& imagePath,
	                               const std::vector<std::wstring>& extensions) const
	{
		if (imagePath.empty() || extensions.empty()) {
			return {};
		}

		const std::filesystem::path folder = std::filesystem::path{imagePath}.parent_path();
		std::vector<std::wstring> paths = collect(folder, buildExtensionSet(extensions));

		std::sort(paths.begin(), paths.end(), namePrecedes);
		return paths;
	}
};

// -----------------------------------------------------------------------------
// OFolderImageScanner Implementation
// -----------------------------------------------------------------------------
OFolderImageScanner::OFolderImageScanner() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OFolderImageScanner::~OFolderImageScanner() noexcept = default;
// -----------------------------------------------------------------------------
OFolderImageScanner& OFolderImageScanner::getInstance()
{
	static OFolderImageScanner s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OFolderImageScanner::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OFolderImageScanner::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OFolderImageScanner::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
std::vector<std::wstring> OFolderImageScanner::scan(const std::wstring& imagePath,
                                                    const std::vector<std::wstring>& extensions) const
{
	return m_pImpl->scan(imagePath, extensions);
}
// -----------------------------------------------------------------------------
