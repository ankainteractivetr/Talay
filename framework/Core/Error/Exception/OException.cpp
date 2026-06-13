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
#include "framework/Core/Error/Exception/OException.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstdint>
#include <functional>
#include <memory>

using namespace anka::Core::Error::Exception;
using namespace anka::Core::Object;

// -----------------------------------------------------------------------------
// OException::Impl Definition
// -----------------------------------------------------------------------------
struct OException::Impl
{
	ObjectIdentity        m_identity;
	std::wstring          m_errorMessage;
	std::function<void()> m_recoveryFunction;

	Impl() :
		m_identity {},
		m_errorMessage {},
		m_recoveryFunction {}
	{}

	Impl(const std::wstring& objectName,
		  const std::wstring& message,
		  const std::function<void()>& recoveryFunction) :
		m_identity {objectName},
		m_errorMessage {message},
		m_recoveryFunction {recoveryFunction}
	{}

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

	const std::wstring& getExceptionMessage(void) const
	{
		return m_errorMessage;
	}

	void setExceptionMessage(const std::wstring& message)
	{
		m_errorMessage = message;
	}

	const std::function<void()>& getRecoveryFunction(void) const
	{
		return m_recoveryFunction;
	}

	void setRecoveryFunction(const std::function<void()>& function)
	{
		m_recoveryFunction = function;
	}
};

// -----------------------------------------------------------------------------
// OException Implementation
// -----------------------------------------------------------------------------
OException::OException() :
	m_pImpl {std::make_unique<Impl>()}
{ }
// -----------------------------------------------------------------------------
OException::OException(const std::wstring& objectName,
							  const std::wstring& message,
							  const std::function<void()>& recoveryFunction) :
	m_pImpl {std::make_unique<Impl>(objectName, message, recoveryFunction)}
{}
// -----------------------------------------------------------------------------
OException::~OException() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OException::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OException::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OException::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
const std::wstring& OException::getErrorMessage(void) const
{
	return m_pImpl->getExceptionMessage();
}
// -----------------------------------------------------------------------------
void OException::setErrorMessage(const std::wstring& message)
{
	m_pImpl->setExceptionMessage(message);
}
// -----------------------------------------------------------------------------
const std::function<void()>& OException::getRecoveryFunction(void) const
{
	return m_pImpl->getRecoveryFunction();
}
// -----------------------------------------------------------------------------
void OException::setRecoveryFunction(const std::function<void()>& function)
{
	m_pImpl->setRecoveryFunction(function);
}
// -----------------------------------------------------------------------------
