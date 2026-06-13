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
#include "IFile.hpp"
#include "OFile.hpp"
#include "OTextFile.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

using namespace anka::Core::File;

// -----------------------------------------------------------------------------
// Encoding helpers
//
// Text files are stored as UTF-8 (plain, human-editable text). Earlier versions
// dumped raw wchar_t (UTF-16LE) bytes, which made the files unreadable by text
// editors and broke round-tripping of hand-edited files. These helpers convert
// between the in-memory std::wstring (UTF-16) and on-disk UTF-8 bytes, and
// detect/strip the byte-order marks left by either format.
// -----------------------------------------------------------------------------
namespace
{
	std::string wideToUtf8(const std::wstring& text)
	{
		if (text.empty()) {
			return {};
		}

		const int sizeNeeded {::WideCharToMultiByte(
			CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
			nullptr, 0, nullptr, nullptr)};

		if (sizeNeeded <= 0) {
			return {};
		}

		std::string out(static_cast<std::size_t>(sizeNeeded), '\0');
		::WideCharToMultiByte(
			CP_UTF8, 0, text.data(), static_cast<int>(text.size()),
			out.data(), sizeNeeded, nullptr, nullptr);
		return out;
	}

	std::wstring utf8ToWide(const char* data, std::size_t size)
	{
		if (data == nullptr || size == 0) {
			return {};
		}

		const int sizeNeeded {::MultiByteToWideChar(
			CP_UTF8, 0, data, static_cast<int>(size), nullptr, 0)};

		if (sizeNeeded <= 0) {
			return {};
		}

		std::wstring out(static_cast<std::size_t>(sizeNeeded), L'\0');
		::MultiByteToWideChar(
			CP_UTF8, 0, data, static_cast<int>(size), out.data(), sizeNeeded);
		return out;
	}

	bool hasUtf16LeBom(const std::vector<unsigned char>& bytes)
	{
		return bytes.size() >= 2 && bytes[0] == 0xFF && bytes[1] == 0xFE;
	}

	// Legacy UTF-16LE files (written by earlier versions) carried a BOM.
	std::wstring decodeUtf16Le(const std::vector<unsigned char>& bytes)
	{
		const std::size_t count {(bytes.size() - 2) / sizeof(wchar_t)};
		return std::wstring(reinterpret_cast<const wchar_t*>(bytes.data() + 2), count);
	}

	std::size_t utf8BomOffset(const std::vector<unsigned char>& bytes)
	{
		const bool hasBom {bytes.size() >= 3 &&
			bytes[0] == 0xEF && bytes[1] == 0xBB && bytes[2] == 0xBF};
		return hasBom ? std::size_t {3} : std::size_t {0};
	}
}

// -----------------------------------------------------------------------------
// OTextFile::Impl Definition
// -----------------------------------------------------------------------------
struct OTextFile::Impl
{
	std::unique_ptr<IFile> m_fileObject;

	Impl() :
		m_fileObject {std::make_unique<OFile>()}
	{}

	Impl(const std::wstring& objectName,
		  const std::wstring& fileName,
		  const std::filesystem::path& filePath) :
		m_fileObject {std::make_unique<OFile>(objectName, fileName, filePath)}
	{}

	const std::wstring& getObjectName(void) const
	{
		return m_fileObject->getName();
	}

	void setObjectName(const std::wstring& name) const
	{
		m_fileObject->setName(name);
	}

	std::uint64_t getObjectId(void) const
	{
		return m_fileObject->getId();
	}

	const std::wstring& getFileName(void) const
	{
		return m_fileObject->getFileName();
	}

	void setFileName(const std::wstring& fileName) const
	{
		m_fileObject->setFileName(fileName);
	}

	const std::filesystem::path& getFilePath(void) const
	{
		return m_fileObject->getFilePath();
	}

	void setFilePath(const std::filesystem::path& path) const
	{
		m_fileObject->setFilePath(path);
	}

	bool openFile(void) const
	{
		return m_fileObject->open();
	}

	bool isFileOpen(void) const
	{
		return m_fileObject->isOpen();
	}

	bool closeFile(void) const
	{
		return m_fileObject->close();
	}

	bool writeToFile(const void* buffer, std::size_t size) const
	{
		if (isFileOpen()) {
			return m_fileObject->write(static_cast<const char*>(buffer), size);
		}

		return false;
	}

	bool appendTextToFile(const std::wstring& text) const
	{
		const std::string utf8 {wideToUtf8(text)};
		return writeToFile(utf8.data(), utf8.size());
	}

	bool truncateFile(void) const
	{
		return m_fileObject->truncate();
	}

	std::vector<unsigned char> readAllBytes(void)
	{
		return m_fileObject->readAll();
	}

	std::wstring readAllTextFromFile(void)
	{
		const auto bytes {m_fileObject->readAll()};

		if (bytes.empty()) {
			return {};
		}

		if (hasUtf16LeBom(bytes)) {
			return decodeUtf16Le(bytes);
		}

		const std::size_t offset {utf8BomOffset(bytes)};
		return utf8ToWide(reinterpret_cast<const char*>(bytes.data() + offset),
								bytes.size() - offset);
	}
};

// -----------------------------------------------------------------------------
// OTextFile Implementation
// -----------------------------------------------------------------------------
OTextFile::OTextFile() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OTextFile::OTextFile(const std::wstring& objectName,
							const std::wstring& fileName,
							const std::filesystem::path& filePath) :
	m_pImpl {std::make_unique<Impl>(objectName, fileName, filePath)}
{}
// -----------------------------------------------------------------------------
OTextFile::~OTextFile() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OTextFile::getFileName(void) const
{
	return m_pImpl->getFileName();
}
// -----------------------------------------------------------------------------
void OTextFile::setFileName(const std::wstring& fileName)
{
	m_pImpl->setFileName(fileName);
}
// -----------------------------------------------------------------------------
const std::filesystem::path& OTextFile::getFilePath(void) const
{
	return m_pImpl->getFilePath();
}
// -----------------------------------------------------------------------------
void OTextFile::setFilePath(const std::filesystem::path& path)
{
	m_pImpl->setFilePath(path);
}
// -----------------------------------------------------------------------------
const std::wstring& OTextFile::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OTextFile::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OTextFile::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OTextFile::open(void)
{
	return m_pImpl->openFile();
}
// -----------------------------------------------------------------------------
bool OTextFile::isOpen(void) const
{
	return m_pImpl->isFileOpen();
}
// -----------------------------------------------------------------------------
bool OTextFile::close(void)
{
	return m_pImpl->closeFile();
}
// -----------------------------------------------------------------------------
bool OTextFile::write(const void* buffer, std::size_t size)
{
	return m_pImpl->writeToFile(buffer, size);
}
// -----------------------------------------------------------------------------
bool OTextFile::truncate(void)
{
	return m_pImpl->truncateFile();
}
// -----------------------------------------------------------------------------
std::vector<unsigned char> OTextFile::readAll(void)
{
	return m_pImpl->readAllBytes();
}
// -----------------------------------------------------------------------------
bool OTextFile::appendText(const std::wstring& text)
{
	return m_pImpl->appendTextToFile(text);
}
// -----------------------------------------------------------------------------
std::wstring OTextFile::readAllText(void)
{
	return m_pImpl->readAllTextFromFile();
}
// -----------------------------------------------------------------------------
