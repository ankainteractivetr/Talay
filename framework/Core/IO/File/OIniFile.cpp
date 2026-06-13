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
#include "OTextFile.hpp"
#include "OIniFile.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace anka::Core::File;

// -----------------------------------------------------------------------------
// Local helpers — pure parsing primitives over a single trimmed INI line.
// -----------------------------------------------------------------------------
namespace
{
	std::wstring trim(const std::wstring& s)
	{
		const auto first {s.find_first_not_of(L" \t\r\n")};
		if (first == std::wstring::npos) {
			return {};
		}

		const auto last {s.find_last_not_of(L" \t\r\n")};
		return s.substr(first, last - first + 1);
	}

	bool isComment(const std::wstring& line)
	{
		return line.empty() || line[0] == L';' || line[0] == L'#';
	}

	bool isSectionHeader(const std::wstring& line)
	{
		return line.size() >= 3 && line.front() == L'[' && line.back() == L']';
	}

	std::wstring sectionNameOf(const std::wstring& line)
	{
		return trim(line.substr(1, line.size() - 2));
	}

	// Splits "key = value" into a trimmed pair; false if there is no key.
	bool parseKeyValue(const std::wstring& line, IniPair& out)
	{
		const auto eqPos {line.find(L'=')};
		if (eqPos == std::wstring::npos) {
			return false;
		}

		std::wstring key {trim(line.substr(0, eqPos))};
		if (key.empty()) {
			return false;
		}

		out = IniPair {std::move(key), trim(line.substr(eqPos + 1))};
		return true;
	}

	// Returns the next '\n'-delimited, trimmed line and advances pos past it.
	std::wstring nextLine(const std::wstring& content, std::size_t& pos)
	{
		std::size_t lineEnd {content.find(L'\n', pos)};
		if (lineEnd == std::wstring::npos) {
			lineEnd = content.size();
		}

		std::wstring line {trim(content.substr(pos, lineEnd - pos))};
		pos = lineEnd + 1;
		return line;
	}
}

// -----------------------------------------------------------------------------
// OIniFile::Impl Definition
// -----------------------------------------------------------------------------
struct OIniFile::Impl
{
	std::unique_ptr<ITextFile> m_fileObject;
	IniSections                m_sections;

	Impl() :
		m_fileObject {std::make_unique<OTextFile>()}
	{}

	Impl(const std::wstring& objectName,
		  const std::wstring& fileName,
		  const std::filesystem::path& filePath) :
		m_fileObject {std::make_unique<OTextFile>(objectName, fileName, filePath)}
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
		if (isFileOpen()) {
			return m_fileObject->appendText(text);
		}

		return false;
	}

	// Routes one trimmed line into the model: comment, section header, or pair.
	void consumeLine(const std::wstring& line, std::wstring& currentSection)
	{
		if (isComment(line)) {
			return;
		}
		if (isSectionHeader(line)) {
			currentSection = sectionNameOf(line);
			m_sections.try_emplace(currentSection);
			return;
		}
		addPair(line, currentSection);
	}

	void addPair(const std::wstring& line, const std::wstring& currentSection)
	{
		IniPair pair;
		if (!currentSection.empty() && parseKeyValue(line, pair)) {
			m_sections[currentSection].push_back(std::move(pair));
		}
	}

	void parseContent(const std::wstring& content)
	{
		m_sections.clear();
		std::wstring currentSection;
		std::size_t  pos {0};

		while (pos < content.size()) {
			consumeLine(nextLine(content, pos), currentSection);
		}
	}

	bool readSections()
	{
		if (!openFile()) {
			return false;
		}

		const std::wstring content {m_fileObject->readAllText()};
		closeFile();
		parseContent(content);
		return true;
	}

	void emitSection(const std::wstring& section, const std::vector<IniPair>& pairs)
	{
		appendTextToFile(L"[" + section + L"]\n");
		for (const auto& [key, val] : pairs) {
			appendTextToFile(key + L"=" + val + L"\n");
		}
		appendTextToFile(L"\n");
	}

	bool writeSections()
	{
		if (!openFile()) {
			return false;
		}
		if (!m_fileObject->truncate()) {
			closeFile();
			return false;
		}

		for (const auto& [section, pairs] : m_sections) {
			emitSection(section, pairs);
		}

		closeFile();
		return true;
	}

	std::wstring getValue(const std::wstring& sectionName, const std::wstring& valueName)
	{
		const auto it = m_sections.find(sectionName);
		if (it == m_sections.end()) {
			return {};
		}
		for (const auto& [key, val] : it->second) {
			if (key == valueName) {
				return val;
			}
		}
		return {};
	}

	bool setValue(const std::wstring& sectionName, const std::wstring& valueName, const std::wstring& value)
	{
		auto& pairs = m_sections[sectionName];
		for (auto& [key, val] : pairs) {
			if (key == valueName) {
				val = value;
				return writeSections();
			}
		}
		pairs.emplace_back(valueName, value);
		return writeSections();
	}

	// A generic INI file has no app-specific schema to seed, so "defaults" is just
	// an empty file. The application's settings layer is what knows its sections
	// and supplies their fallback values.
	bool generateDefaults()
	{
		m_sections.clear();
		return writeSections();
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
		return m_fileObject->readAllText();
	}
};
// -----------------------------------------------------------------------------
// OIniFile Implementation
// -----------------------------------------------------------------------------
OIniFile::OIniFile() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OIniFile::OIniFile(const std::wstring& objectName,
					    const std::wstring& fileName,
						 const std::filesystem::path& filePath) :
	m_pImpl {std::make_unique<Impl>(objectName, fileName, filePath)}
{}
// -----------------------------------------------------------------------------
OIniFile::~OIniFile() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OIniFile::getFileName(void) const
{
	return m_pImpl->getFileName();
}
// -----------------------------------------------------------------------------
void OIniFile::setFileName(const std::wstring& fileName)
{
	m_pImpl->setFileName(fileName);
}
// -----------------------------------------------------------------------------
const std::filesystem::path& OIniFile::getFilePath(void) const
{
	return m_pImpl->getFilePath();
}
// -----------------------------------------------------------------------------
void OIniFile::setFilePath(const std::filesystem::path& path)
{
	m_pImpl->setFilePath(path);
}
// -----------------------------------------------------------------------------
const std::wstring& OIniFile::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OIniFile::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OIniFile::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OIniFile::open(void)
{
	return m_pImpl->openFile();
}
// -----------------------------------------------------------------------------
bool OIniFile::isOpen(void) const
{
	return m_pImpl->isFileOpen();
}
// -----------------------------------------------------------------------------
bool OIniFile::close(void)
{
	return m_pImpl->closeFile();
}
// -----------------------------------------------------------------------------
bool OIniFile::write(const void* buffer, std::size_t size)
{
	return m_pImpl->writeToFile(buffer, size);
}
// -----------------------------------------------------------------------------
bool OIniFile::truncate(void)
{
	return m_pImpl->truncateFile();
}
// -----------------------------------------------------------------------------
std::vector<unsigned char> OIniFile::readAll(void)
{
	return m_pImpl->readAllBytes();
}
// -----------------------------------------------------------------------------
std::wstring OIniFile::readAllText(void)
{
	return m_pImpl->readAllTextFromFile();
}
// -----------------------------------------------------------------------------
bool OIniFile::appendText(const std::wstring& text)
{
	return m_pImpl->appendTextToFile(text);
}
// -----------------------------------------------------------------------------
bool OIniFile::readSections()
{
	return m_pImpl->readSections();
}
// -----------------------------------------------------------------------------
bool OIniFile::writeSections()
{
	return m_pImpl->writeSections();
}
// -----------------------------------------------------------------------------
std::wstring OIniFile::getValue(const std::wstring& sectionName, const std::wstring& valueName)
{
	return m_pImpl->getValue(sectionName, valueName);
}
// -----------------------------------------------------------------------------
bool OIniFile::setValue(const std::wstring& sectionName, const std::wstring& valueName, const std::wstring& value)
{
	return m_pImpl->setValue(sectionName, valueName, value);
}
// -----------------------------------------------------------------------------
bool OIniFile::generateDefaults()
{
	return m_pImpl->generateDefaults();
}
// -----------------------------------------------------------------------------
