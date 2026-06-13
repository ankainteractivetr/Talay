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
#include "OObject.hpp"

#include <cstdint>
#include <string>

using namespace anka::Core::Object;

// OObject is a thin IObject adapter over an embedded ObjectIdentity, which is
// the single owner of name/id storage and the id counter. The copy/move members
// are defaulted: ObjectIdentity already mints a fresh id on copy and carries the
// id on move, which is exactly OObject's intended identity semantics.

// -----------------------------------------------------------------------------
// OObject Implementation
// -----------------------------------------------------------------------------
OObject::OObject() :
   m_identity {}
{}
// -----------------------------------------------------------------------------
OObject::OObject(const std::wstring& name) :
   m_identity {name}
{}
// -----------------------------------------------------------------------------
OObject::OObject(const OObject& object) = default;
// -----------------------------------------------------------------------------
OObject::OObject(OObject&& object) noexcept = default;
// -----------------------------------------------------------------------------
OObject::~OObject() noexcept = default;
// -----------------------------------------------------------------------------
OObject& OObject::operator=(const OObject& object) = default;
// -----------------------------------------------------------------------------
OObject& OObject::operator=(OObject&& object) noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OObject::getName(void) const
{
   return m_identity.name();
}
// -----------------------------------------------------------------------------
void OObject::setName(const std::wstring& name)
{
   m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OObject::getId(void) const
{
   return m_identity.id();
}
// -----------------------------------------------------------------------------
