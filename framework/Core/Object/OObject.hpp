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

#ifndef ANKA_CORE_OBJECT_OOBJECT_HPP
#define ANKA_CORE_OBJECT_OOBJECT_HPP

#include "IObject.hpp"
#include "ObjectIdentity.hpp"

#include <cstdint>
#include <string>

namespace anka
{
	namespace Core
	{
		namespace Object
		{
			/*
			*
			* Concrete Object Class
			*
			*/
			class OObject final : public IObject
			{
				public:

					//
					// COTORs and DOTOR
					//

					OObject();
					explicit OObject(const std::wstring& name);
					OObject(const OObject& object);
					OObject(OObject&& object) noexcept;

					~OObject() noexcept override;

					//
					// Operators
					//

					OObject& operator=(const OObject& object);
					OObject& operator=(OObject&& object) noexcept;

					//
					// Getters and Setters
					//

					const std::wstring& getName(void) const override;
					void                setName(const std::wstring& name) override;

					std::uint64_t       getId(void) const override;


				private:

					ObjectIdentity m_identity;
			};
		}
	}
}

#endif // ANKA_CORE_OBJECT_OOBJECT_HPP