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

#ifndef ANKA_CORE_OBJECT_OBJECTIDENTITY_HPP
#define ANKA_CORE_OBJECT_OBJECTIDENTITY_HPP

#include <atomic>
#include <cstdint>
#include <string>
#include <utility>

namespace anka
{
	namespace Core
	{
		namespace Object
		{
			// Single source of truth for an object's identity: a human-readable
			// name plus a process-unique, monotonically increasing id. Every
			// framework object embeds one of these instead of re-implementing
			// name/id storage, so the id counter lives in exactly one place.
			//
			// Identity is per-instance: a copy is a *new* object and gets a fresh
			// id, while a move carries the existing id (and name) across. This
			// preserves the historical OObject semantics.
			class ObjectIdentity
			{
				public:

					ObjectIdentity() :
						m_name {},
						m_id {nextId()}
					{}

					explicit ObjectIdentity(std::wstring name) :
						m_name {std::move(name)},
						m_id {nextId()}
					{}

					ObjectIdentity(const ObjectIdentity& other) :
						m_name {other.m_name},
						m_id {nextId()}
					{}

					ObjectIdentity(ObjectIdentity&& other) noexcept :
						m_name {std::move(other.m_name)},
						m_id {other.m_id}
					{}

					ObjectIdentity& operator=(const ObjectIdentity& other)
					{
						if (this != &other) {
							m_name = other.m_name;
							m_id   = nextId();
						}

						return *this;
					}

					ObjectIdentity& operator=(ObjectIdentity&& other) noexcept
					{
						if (this != &other) {
							m_name = std::move(other.m_name);
							m_id   = other.m_id;
						}

						return *this;
					}

					const std::wstring& name(void) const
					{
						return m_name;
					}

					void setName(const std::wstring& name)
					{
						m_name = name;
					}

					std::uint64_t id(void) const
					{
						return m_id;
					}

				private:

					static std::uint64_t nextId(void)
					{
						static std::atomic<std::uint64_t> s_counter {0};
						return ++s_counter;
					}

					std::wstring  m_name;
					std::uint64_t m_id;
			};
		}
	}
}

#endif // ANKA_CORE_OBJECT_OBJECTIDENTITY_HPP
