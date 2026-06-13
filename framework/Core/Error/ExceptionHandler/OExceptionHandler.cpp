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
#include "OExceptionHandler.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Core/Error/Exception/IException.hpp"

using namespace anka::Core::Error::ExceptionHandler;
using namespace anka::Core::Error::Exception;
using namespace anka::Core::Object;

// -----------------------------------------------------------------------------
// OExceptionHandler::Impl Definition
// -----------------------------------------------------------------------------
struct OExceptionHandler::Impl
{
	ObjectIdentity m_identity {L"ExceptionHandlerObject"};

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

	// Stubs for now — error reporting/recovery is not yet implemented.
	bool handleException([[maybe_unused]] const std::exception& exception) const
	{
		return true;
	}

	bool handleException([[maybe_unused]] const std::shared_ptr<IException> exception) const
	{
		return true;
	}
};

// -----------------------------------------------------------------------------
// OExceptionHandler Implementation
// -----------------------------------------------------------------------------
OExceptionHandler::OExceptionHandler() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OExceptionHandler::~OExceptionHandler() noexcept = default;
// -----------------------------------------------------------------------------
OExceptionHandler& OExceptionHandler::getInstance()
{
	static OExceptionHandler s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OExceptionHandler::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OExceptionHandler::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OExceptionHandler::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OExceptionHandler::handleException(const std::exception& exception) const
{
	return m_pImpl->handleException(exception);
}
// -----------------------------------------------------------------------------
bool OExceptionHandler::handleException(const std::shared_ptr<IException> exception) const
{
	return m_pImpl->handleException(exception);
}
// -----------------------------------------------------------------------------
