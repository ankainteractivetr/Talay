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
#include "OFile.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <fstream>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>

using namespace anka::Core::File;
using namespace anka::Core::Object;

// -----------------------------------------------------------------------------
// OFile::Impl Definition
// -----------------------------------------------------------------------------
struct OFile::Impl
{
	ObjectIdentity        m_identity;
	std::wstring          m_fileName;
	std::filesystem::path m_filePath;
	std::fstream          m_file;

	Impl() :
		m_identity {},
		m_fileName {},
		m_filePath {}
	{}

	Impl(const std::wstring& objectName,
		  const std::wstring& fileName,
		  const std::filesystem::path& filePath) :
		m_identity {objectName},
		m_fileName {fileName},
		m_filePath {filePath}
	{}

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

	const std::wstring& getFileName(void) const
	{
		return m_fileName;
	}

	void setFileName(const std::wstring& fileName)
	{
		m_fileName = fileName;
	}

	const std::filesystem::path& getFilePath(void)
	{
		return m_filePath;
	}

	void setFilePath(const std::filesystem::path& pathName)
	{
		m_filePath = pathName;
	}

	bool openFile(void)
	{
		if (!isFileOpen()) {
			if (createDirectoriesIfNecessary()) {
				openTheActualFile();
			}
		}

		return isFileOpen();
	}

	bool isFileOpen(void) const
	{
		return m_file.is_open();
	}

	bool closeFile(void)
	{
		if (isFileOpen()) {
			m_file.close();
		}

		return !isFileOpen();
	}

	bool writeToFile(const void* buffer, std::size_t size)
	{
		if (isFileOpen()) {
			m_file.write(static_cast<const char*>(buffer), size);
			return !m_file.bad();
		}

		return false;
	}

	bool truncateFile(void)
	{
		if (!isFileOpen()) {
			return false;
		}

		const std::filesystem::path fullPath {m_filePath / m_fileName};
		m_file.close();
		m_file.open(fullPath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
		return isFileOpen();
	}

	std::vector<unsigned char> readAllBytes(void)
	{
		if (!isFileOpen()) {
			return {};
		}

		m_file.flush();

		const std::filesystem::path fullPath {m_filePath / m_fileName};
		const auto fileSize {std::filesystem::file_size(fullPath)};

		if (fileSize == 0) {
			return {};
		}

		m_file.seekg(0, std::ios::beg);

		std::vector<unsigned char> buffer(static_cast<std::size_t>(fileSize));
		m_file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
		buffer.resize(static_cast<std::size_t>(m_file.gcount()));
		return buffer;
	}

	bool createDirectoriesIfNecessary(void) const
	{
		if (!m_filePath.empty() && !std::filesystem::exists(m_filePath)) {
			if (!std::filesystem::create_directories(m_filePath)) {
				return false;
			}
		}

		return true;
	}

	bool openTheActualFile(void)
	{
		const std::filesystem::path fullPath {m_filePath / m_fileName};

		if (std::filesystem::exists(fullPath)) {
			m_file.open(fullPath, std::ios::binary | std::ios::in | std::ios::out | std::ios::ate);
		} else {
			m_file.open(fullPath, std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc);
		}

		return m_file.is_open();
	}
};

// -----------------------------------------------------------------------------
// OFile Implementation
// -----------------------------------------------------------------------------
OFile::OFile() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OFile::OFile(const std::wstring& objectName,
				 const std::wstring& fileName,
				 const std::filesystem::path& filePath) :
	m_pImpl {std::make_unique<Impl>(objectName, fileName, filePath)}
{}
// -----------------------------------------------------------------------------
OFile::~OFile() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OFile::getFileName(void) const
{
	return m_pImpl->getFileName();
}
// -----------------------------------------------------------------------------
void OFile::setFileName(const std::wstring& fileName)
{
	m_pImpl->setFileName(fileName);
}
// -----------------------------------------------------------------------------
const std::filesystem::path& OFile::getFilePath(void) const
{
	return m_pImpl->getFilePath();
}
// -----------------------------------------------------------------------------
void OFile::setFilePath(const std::filesystem::path& path)
{
	m_pImpl->setFilePath(path);
}
// -----------------------------------------------------------------------------
const std::wstring& OFile::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OFile::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OFile::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OFile::open(void)
{
	return m_pImpl->openFile();
}
// -----------------------------------------------------------------------------
bool OFile::isOpen(void) const
{
	return m_pImpl->isFileOpen();
}
// -----------------------------------------------------------------------------
bool OFile::close(void)
{
	return m_pImpl->closeFile();
}
// -----------------------------------------------------------------------------
bool OFile::write(const void* buffer, std::size_t size)
{
	return m_pImpl->writeToFile(buffer, size);
}
// -----------------------------------------------------------------------------
bool OFile::truncate(void)
{
	return m_pImpl->truncateFile();
}
// -----------------------------------------------------------------------------
std::vector<unsigned char> OFile::readAll(void)
{
	return m_pImpl->readAllBytes();
}
// -----------------------------------------------------------------------------
