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

#ifndef TALAY_SETTINGS_OTALAYSETTINGS_HPP
#define TALAY_SETTINGS_OTALAYSETTINGS_HPP

#include "ITalaySettings.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace talay
{
	namespace Settings
	{
		/*
		*
		* Concrete OTalaySettings Class
		*
		*/
		class OTalaySettings final : public ITalaySettings
		{
			public:

				OTalaySettings();
				OTalaySettings(const std::wstring& objectName,
				               const std::filesystem::path& settingsPath);
				OTalaySettings(const OTalaySettings& object) = delete;
				OTalaySettings(OTalaySettings&& object) noexcept = delete;

				~OTalaySettings() noexcept override;

				OTalaySettings& operator=(const OTalaySettings& object) = delete;
				OTalaySettings& operator=(OTalaySettings&& object) noexcept = delete;

				//
				// IObject overrides
				//

				const std::wstring& getName(void) const override;
				void                setName(const std::wstring& name) override;

				std::uint64_t getId(void) const override;

				//
				// ITalaySettings overrides
				//

				bool load(void) override;

				bool save(void) override;

				void resetToDefaults(void) override;

				//
				// General
				//

				std::optional<talay::Localization::Language> getLanguage(void) const override;
				void setLanguage(std::optional<talay::Localization::Language> language) override;

				//
				// MainWindow
				//

				int  getMainWindowPosX(void) const override;
				void setMainWindowPosX(int value) override;

				int  getMainWindowPosY(void) const override;
				void setMainWindowPosY(int value) override;

				int  getMainWindowClientWidth(void) const override;
				void setMainWindowClientWidth(int value) override;

				int  getMainWindowClientHeight(void) const override;
				void setMainWindowClientHeight(int value) override;

				bool isAlwaysOnTopEnabled(void) const override;
				void setAlwaysOnTopEnabled(bool enabled) override;

				//
				// Library
				//

				bool isAutoAddToLibraryEnabled(void) const override;
				void setAutoAddToLibraryEnabled(bool enabled) override;

				//
				// Shell
				//

				bool isFileTypesRegistrationEnabled(void) const override;
				void setFileTypesRegistrationEnabled(bool enabled) override;

				//
				// Conversion
				//

				std::wstring getConversionFormat(void) const override;
				void         setConversionFormat(const std::wstring& extension) override;

				std::wstring getConversionOutputDirectory(void) const override;
				void         setConversionOutputDirectory(const std::wstring& directory) override;


			private:

				struct Impl;
				std::unique_ptr<Impl> m_pImpl;
		};
	}
}

#endif // TALAY_SETTINGS_OTALAYSETTINGS_HPP
