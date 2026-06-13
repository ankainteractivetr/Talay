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

#ifndef ANKA_CORE_SETTINGS_IPROGRAMSETTINGS_HPP
#define ANKA_CORE_SETTINGS_IPROGRAMSETTINGS_HPP

#include "framework/Core/Object/IObject.hpp"

#include <string>

namespace anka
{
	namespace Core
	{
		namespace Settings
		{
			/*
			*
			* Abstract Program Settings Interface
			*
			*/
			class IProgramSettings : public anka::Core::Object::IObject
			{
				public:

					virtual ~IProgramSettings() noexcept override = default;

					//
					// Load / Save
					//

					virtual bool load(void) = 0;

					virtual bool save(void) = 0;

					//
					// Key / Value Access
					//

					virtual std::wstring getValue(const std::wstring& sectionName,
					                              const std::wstring& keyName) = 0;

					virtual bool setValue(const std::wstring& sectionName,
					                      const std::wstring& keyName,
					                      const std::wstring& value) = 0;
			};
		}
	}
}

#endif // ANKA_CORE_SETTINGS_IPROGRAMSETTINGS_HPP
