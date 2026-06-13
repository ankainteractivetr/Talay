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
#include "OTalaySettings.hpp"
#include "framework/Core/Settings/OProgramSettings.hpp"
#include "framework/Core/IO/Folder/KnownFolders.hpp"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

using namespace talay::Settings;
using namespace anka::Core::Settings;
using talay::Localization::Language;

// -----------------------------------------------------------------------------
// Factory defaults — the values a missing or first-run ini is seeded with, in
// one place so the member fallbacks and the on-disk defaults agree.
// -----------------------------------------------------------------------------
namespace
{
   constexpr int k_defaultPosX         {170};
   constexpr int k_defaultPosY         {180};
   constexpr int k_defaultClientWidth  {1280};
   constexpr int k_defaultClientHeight {720};

   constexpr wchar_t k_defaultConversionFormat[] {L"png"};

   // Convert output defaults to the Desktop; empty if the shell can't resolve
   // it, leaving the conversion window's own source-folder fallback to take over.
   std::wstring defaultOutputDirectory(void)
   {
      return anka::Core::IO::KnownFolders::desktop().wstring();
   }

   std::wstring nonEmptyOr(std::wstring value, const std::wstring& fallback)
   {
      return value.empty() ? fallback : value;
   }
}

// -----------------------------------------------------------------------------
// OTalaySettings::Impl Definition
// -----------------------------------------------------------------------------
struct OTalaySettings::Impl
{
	OProgramSettings m_settings;

	int m_mainWindowPosX         {k_defaultPosX};
	int m_mainWindowPosY         {k_defaultPosY};
	int m_mainWindowClientWidth  {k_defaultClientWidth};
	int m_mainWindowClientHeight {k_defaultClientHeight};

	std::wstring m_conversionFormat    {k_defaultConversionFormat};
	std::wstring m_conversionOutputDir;

	std::optional<Language> m_languagePref {std::nullopt};

	bool m_autoAddToLibrary {true};
	bool m_alwaysOnTop {false};
	bool m_fileTypesRegistered {true};

	Impl() :
		m_settings {}
	{}

	Impl(const std::wstring& objectName,
		  const std::filesystem::path& settingsPath) :
		m_settings {objectName, settingsPath}
	{}

	const std::wstring& getObjectName(void) const
	{
		return m_settings.getName();
	}

	void setObjectName(const std::wstring& name)
	{
		m_settings.setName(name);
	}

	std::uint64_t getObjectId(void) const
	{
		return m_settings.getId();
	}

	static int parseValue(const std::wstring& s, int fallback)
	{
		if (s.empty()) {
			return fallback;
		}

		try {
			return std::stoi(s);
		} catch (...) {
			return fallback;
		}
	}

	// [General] language: "0" -> Turkish, "1" -> English, anything else (incl.
	// absent) -> empty, meaning "no choice saved, fall back to the OS language".
	static std::optional<Language> parseLanguage(const std::wstring& s)
	{
		if (s == L"0") return Language::Turkish;
		if (s == L"1") return Language::English;
		return std::nullopt;
	}

	// Boolean keys: "0" / "1" are honoured, anything else (incl. absent) is
	// the given default — a hand-mangled ini never flips a setting silently.
	static bool parseFlag(const std::wstring& s, bool fallback)
	{
		if (s == L"0") return false;
		if (s == L"1") return true;
		return fallback;
	}

	// The ini token for a language choice: 0 / 1 mirror parseLanguage, empty
	// means "no choice saved, follow the OS".
	static std::wstring languageToken(std::optional<Language> language)
	{
		if (!language)
			return L"";

		return *language == Language::Turkish ? L"0" : L"1";
	}

	void applyDefaults(void)
	{
		m_settings.setValue(L"General",    L"language",      languageToken(m_languagePref));
		m_settings.setValue(L"MainWindow", L"pos_x",         std::to_wstring(m_mainWindowPosX));
		m_settings.setValue(L"MainWindow", L"pos_y",         std::to_wstring(m_mainWindowPosY));
		m_settings.setValue(L"MainWindow", L"client_width",  std::to_wstring(m_mainWindowClientWidth));
		m_settings.setValue(L"MainWindow", L"client_height", std::to_wstring(m_mainWindowClientHeight));
		m_settings.setValue(L"MainWindow", L"always_on_top", m_alwaysOnTop ? L"1" : L"0");
		m_settings.setValue(L"Library",    L"auto_add_opened", m_autoAddToLibrary ? L"1" : L"0");
		m_settings.setValue(L"Shell",      L"register_file_types", m_fileTypesRegistered ? L"1" : L"0");
		m_settings.setValue(L"Conversion", L"format",        m_conversionFormat);
		m_settings.setValue(L"Conversion", L"output_dir",    m_conversionOutputDir);
	}

	// The ini is missing or unreadable (first run, or deleted by hand): keep the
	// built-in defaults in memory and write a complete default file back to disk,
	// so the user always finds one to edit. A failed write is not fatal — the
	// in-memory defaults still make a fully working configuration.
	bool regenerateDefaults(void)
	{
		applyDefaults();
		m_settings.save();
		return true;
	}

	void readWindowBounds(void)
	{
		m_mainWindowPosX         = parseValue(m_settings.getValue(L"MainWindow", L"pos_x"),         k_defaultPosX);
		m_mainWindowPosY         = parseValue(m_settings.getValue(L"MainWindow", L"pos_y"),         k_defaultPosY);
		m_mainWindowClientWidth  = parseValue(m_settings.getValue(L"MainWindow", L"client_width"),  k_defaultClientWidth);
		m_mainWindowClientHeight = parseValue(m_settings.getValue(L"MainWindow", L"client_height"), k_defaultClientHeight);
	}

	void readPreferences(void)
	{
		m_languagePref     = parseLanguage(m_settings.getValue(L"General", L"language"));
		m_alwaysOnTop      = parseFlag(m_settings.getValue(L"MainWindow", L"always_on_top"), false);
		m_autoAddToLibrary = parseFlag(m_settings.getValue(L"Library", L"auto_add_opened"), true);
		m_fileTypesRegistered = parseFlag(m_settings.getValue(L"Shell", L"register_file_types"), true);

		m_conversionFormat    = nonEmptyOr(m_settings.getValue(L"Conversion", L"format"), k_defaultConversionFormat);
		m_conversionOutputDir = nonEmptyOr(m_settings.getValue(L"Conversion", L"output_dir"), defaultOutputDirectory());
	}

	// The ini layer quietly creates a missing file as an empty one and loads it
	// successfully, so a hand-deleted ini surfaces here as missing keys, not as
	// a failed load. Writing the already-parsed values back fills every gap a
	// hand-edit (or an older version's file) left, while keeping each value the
	// user did set. pos_x and always_on_top are the sentinels: pos_x exists in
	// every complete file, always_on_top migrates pre-feature files once.
	void healMissingKeys(void)
	{
		if (!m_settings.getValue(L"MainWindow", L"pos_x").empty()
		 && !m_settings.getValue(L"MainWindow", L"always_on_top").empty())
			return;

		applyDefaults();
		m_settings.save();
	}

	bool loadSettings(void)
	{
		if (!m_settings.load())
			return regenerateDefaults();

		readWindowBounds();
		readPreferences();
		healMissingKeys();
		return true;
	}

	bool saveSettings(void)
	{
		return m_settings.save();
	}

	// Reset everything to the factory values declared above — the same ones a
	// first-run ini is seeded with — then write the complete default file back,
	// so the conversion output directory included, nothing user-set survives.
	void resetToDefaults(void)
	{
		restoreDefaultMembers();

		applyDefaults();
		m_settings.save();
	}

	void restoreDefaultMembers(void)
	{
		m_languagePref           = std::nullopt;
		m_mainWindowPosX         = k_defaultPosX;
		m_mainWindowPosY         = k_defaultPosY;
		m_mainWindowClientWidth  = k_defaultClientWidth;
		m_mainWindowClientHeight = k_defaultClientHeight;
		m_alwaysOnTop            = false;
		m_autoAddToLibrary       = true;
		m_fileTypesRegistered    = true;
		m_conversionFormat       = k_defaultConversionFormat;
		m_conversionOutputDir    = defaultOutputDirectory();
	}
};
// -----------------------------------------------------------------------------
// OTalaySettings Implementation
// -----------------------------------------------------------------------------
OTalaySettings::OTalaySettings() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OTalaySettings::OTalaySettings(const std::wstring& objectName,
                                 const std::filesystem::path& settingsPath) :
	m_pImpl {std::make_unique<Impl>(objectName, settingsPath)}
{}
// -----------------------------------------------------------------------------
OTalaySettings::~OTalaySettings() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OTalaySettings::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OTalaySettings::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OTalaySettings::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OTalaySettings::load(void)
{
	return m_pImpl->loadSettings();
}
// -----------------------------------------------------------------------------
bool OTalaySettings::save(void)
{
	return m_pImpl->saveSettings();
}
// -----------------------------------------------------------------------------
void OTalaySettings::resetToDefaults(void)
{
	m_pImpl->resetToDefaults();
}
// -----------------------------------------------------------------------------
std::optional<Language> OTalaySettings::getLanguage(void) const
{
	return m_pImpl->m_languagePref;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setLanguage(std::optional<Language> language)
{
	m_pImpl->m_languagePref = language;
	m_pImpl->m_settings.setValue(L"General", L"language", Impl::languageToken(language));
}
// -----------------------------------------------------------------------------
int OTalaySettings::getMainWindowPosX(void) const
{
	return m_pImpl->m_mainWindowPosX;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setMainWindowPosX(int value)
{
	m_pImpl->m_mainWindowPosX = value;
	m_pImpl->m_settings.setValue(L"MainWindow", L"pos_x", std::to_wstring(value));
}
// -----------------------------------------------------------------------------
int OTalaySettings::getMainWindowPosY(void) const
{
	return m_pImpl->m_mainWindowPosY;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setMainWindowPosY(int value)
{
	m_pImpl->m_mainWindowPosY = value;
	m_pImpl->m_settings.setValue(L"MainWindow", L"pos_y", std::to_wstring(value));
}
// -----------------------------------------------------------------------------
int OTalaySettings::getMainWindowClientWidth(void) const
{
	return m_pImpl->m_mainWindowClientWidth;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setMainWindowClientWidth(int value)
{
	m_pImpl->m_mainWindowClientWidth = value;
	m_pImpl->m_settings.setValue(L"MainWindow", L"client_width", std::to_wstring(value));
}
// -----------------------------------------------------------------------------
int OTalaySettings::getMainWindowClientHeight(void) const
{
	return m_pImpl->m_mainWindowClientHeight;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setMainWindowClientHeight(int value)
{
	m_pImpl->m_mainWindowClientHeight = value;
	m_pImpl->m_settings.setValue(L"MainWindow", L"client_height", std::to_wstring(value));
}
// -----------------------------------------------------------------------------
bool OTalaySettings::isAlwaysOnTopEnabled(void) const
{
	return m_pImpl->m_alwaysOnTop;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setAlwaysOnTopEnabled(bool enabled)
{
	m_pImpl->m_alwaysOnTop = enabled;
	m_pImpl->m_settings.setValue(L"MainWindow", L"always_on_top", enabled ? L"1" : L"0");
}
// -----------------------------------------------------------------------------
bool OTalaySettings::isAutoAddToLibraryEnabled(void) const
{
	return m_pImpl->m_autoAddToLibrary;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setAutoAddToLibraryEnabled(bool enabled)
{
	m_pImpl->m_autoAddToLibrary = enabled;
	m_pImpl->m_settings.setValue(L"Library", L"auto_add_opened", enabled ? L"1" : L"0");
}
// -----------------------------------------------------------------------------
bool OTalaySettings::isFileTypesRegistrationEnabled(void) const
{
	return m_pImpl->m_fileTypesRegistered;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setFileTypesRegistrationEnabled(bool enabled)
{
	m_pImpl->m_fileTypesRegistered = enabled;
	m_pImpl->m_settings.setValue(L"Shell", L"register_file_types", enabled ? L"1" : L"0");
}
// -----------------------------------------------------------------------------
std::wstring OTalaySettings::getConversionFormat(void) const
{
	return m_pImpl->m_conversionFormat;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setConversionFormat(const std::wstring& extension)
{
	m_pImpl->m_conversionFormat = extension;
	m_pImpl->m_settings.setValue(L"Conversion", L"format", extension);
}
// -----------------------------------------------------------------------------
std::wstring OTalaySettings::getConversionOutputDirectory(void) const
{
	return m_pImpl->m_conversionOutputDir;
}
// -----------------------------------------------------------------------------
void OTalaySettings::setConversionOutputDirectory(const std::wstring& directory)
{
	m_pImpl->m_conversionOutputDir = directory;
	m_pImpl->m_settings.setValue(L"Conversion", L"output_dir", directory);
}
// -----------------------------------------------------------------------------
