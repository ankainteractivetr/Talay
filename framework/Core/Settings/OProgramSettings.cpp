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
#include "OProgramSettings.hpp"
#include "framework/Core/IO/File/OIniFile.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>

using namespace anka::Core::Settings;
using namespace anka::Core::File;

// -----------------------------------------------------------------------------
// OProgramSettings::Impl Definition
// -----------------------------------------------------------------------------
struct OProgramSettings::Impl
{
	OIniFile m_iniFile;

	Impl() :
		m_iniFile {}
	{}

	Impl(const std::wstring& objectName,
		  const std::filesystem::path& settingsPath) :
		m_iniFile {objectName, L"settings.ini", settingsPath}
	{}

	const std::wstring& getObjectName(void) const
	{
		return m_iniFile.getName();
	}

	void setObjectName(const std::wstring& name)
	{
		m_iniFile.setName(name);
	}

	std::uint64_t getObjectId(void) const
	{
		return m_iniFile.getId();
	}

	bool loadSettings(void)
	{
		return m_iniFile.readSections();
	}

	bool saveSettings(void)
	{
		return m_iniFile.writeSections();
	}

	std::wstring getValueFromSettings(const std::wstring& sectionName,
	                                   const std::wstring& keyName)
	{
		return m_iniFile.getValue(sectionName, keyName);
	}

	bool setValueToSettings(const std::wstring& sectionName,
	                         const std::wstring& keyName,
	                         const std::wstring& value)
	{
		return m_iniFile.setValue(sectionName, keyName, value);
	}
};
// -----------------------------------------------------------------------------
// OProgramSettings Implementation
// -----------------------------------------------------------------------------
OProgramSettings::OProgramSettings() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OProgramSettings::OProgramSettings(const std::wstring& objectName,
                                     const std::filesystem::path& settingsPath) :
	m_pImpl {std::make_unique<Impl>(objectName, settingsPath)}
{}
// -----------------------------------------------------------------------------
OProgramSettings::~OProgramSettings() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OProgramSettings::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OProgramSettings::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OProgramSettings::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OProgramSettings::load(void)
{
	return m_pImpl->loadSettings();
}
// -----------------------------------------------------------------------------
bool OProgramSettings::save(void)
{
	return m_pImpl->saveSettings();
}
// -----------------------------------------------------------------------------
std::wstring OProgramSettings::getValue(const std::wstring& sectionName,
                                          const std::wstring& keyName)
{
	return m_pImpl->getValueFromSettings(sectionName, keyName);
}
// -----------------------------------------------------------------------------
bool OProgramSettings::setValue(const std::wstring& sectionName,
                                  const std::wstring& keyName,
                                  const std::wstring& value)
{
	return m_pImpl->setValueToSettings(sectionName, keyName, value);
}
// -----------------------------------------------------------------------------
