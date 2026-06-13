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

#ifndef ANKA_CORE_SETTINGS_OPROGRAMSETTINGS_HPP
#define ANKA_CORE_SETTINGS_OPROGRAMSETTINGS_HPP

#include "IProgramSettings.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace anka
{
	namespace Core
	{
		namespace Settings
		{
			/*
			*
			* Concrete OProgramSettings Class
			*
			*/
			class OProgramSettings final : public IProgramSettings
			{
				public:

					OProgramSettings();
					OProgramSettings(const std::wstring& objectName,
					                 const std::filesystem::path& settingsPath);
					OProgramSettings(const OProgramSettings& object) = delete;
					OProgramSettings(OProgramSettings&& object) noexcept = delete;

					~OProgramSettings() noexcept override;

					OProgramSettings& operator=(const OProgramSettings& object) = delete;
					OProgramSettings& operator=(OProgramSettings&& object) noexcept = delete;

					//
					// IObject overrides
					//

					const std::wstring& getName(void) const override;
					void                setName(const std::wstring& name) override;

					std::uint64_t getId(void) const override;

					//
					// IProgramSettings overrides
					//

					bool load(void) override;

					bool save(void) override;

					std::wstring getValue(const std::wstring& sectionName,
					                      const std::wstring& keyName) override;

					bool setValue(const std::wstring& sectionName,
					              const std::wstring& keyName,
					              const std::wstring& value) override;


				private:

					struct Impl;
					std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}

#endif // ANKA_CORE_SETTINGS_OPROGRAMSETTINGS_HPP
