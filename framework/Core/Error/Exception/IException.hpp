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

#ifndef ANKA_CORE_ERROR_EXCEPTION_IEXCEPTION_HPP
#define ANKA_CORE_ERROR_EXCEPTION_IEXCEPTION_HPP

#include "framework/Core/Object/IObject.hpp"

#include <string>
#include <functional>

namespace anka
{
	namespace Core
	{
		namespace Error
		{
			namespace Exception
			{
				/*
				* Abstract Exception Interface
				*
				* Exception class to be used in Error Handling
				*
				*/
				class IException : public anka::Core::Object::IObject
				{
					public:

						//
						// COTORs and DOTOR
						//

						virtual ~IException() noexcept override = default;

						//
						// Getters and Setters
						//

						virtual const std::wstring& getErrorMessage(void) const = 0;
						virtual void                setErrorMessage(const std::wstring& name) = 0;

						virtual const std::function<void()>& getRecoveryFunction(void) const = 0;
						virtual void                         setRecoveryFunction(const std::function<void()>& function) = 0;
				};
			}
		}
	}
}

#endif // ANKA_CORE_ERROR_EXCEPTION_IEXCEPTION_HPP
