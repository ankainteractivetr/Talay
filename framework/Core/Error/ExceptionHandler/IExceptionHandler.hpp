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

#ifndef ANKA_CORE_ERROR_EXCEPTIONHANDLER_IEXCEPTIONHANDLER_HPP
#define ANKA_CORE_ERROR_EXCEPTIONHANDLER_IEXCEPTIONHANDLER_HPP

#include "framework/Core/Object/IObject.hpp"

#include <memory>

namespace anka { namespace Core { namespace Error { namespace Exception { class IException; } } } }

namespace anka
{
	namespace Core
	{
		namespace Error
		{
			namespace ExceptionHandler
			{
				/*
				*
				* Abstract Exception Handler Interface
				*
				* Exception Handler class to be used in Error Handling
				*
				*/
				class IExceptionHandler : public anka::Core::Object::IObject
				{
					public:


						//
						// COTORs and DOTOR
						//

						virtual ~IExceptionHandler() noexcept override = default;

						//
						// Other Member Functions
						//

						virtual bool handleException(const std::exception& exception) const = 0;

						virtual bool handleException(const std::shared_ptr<anka::Core::Error::Exception::IException> exception) const = 0;

				};
			}
		}
	}
	
}

#endif // ANKA_CORE_ERROR_EXCEPTIONHANDLER_IEXCEPTIONHANDLER_HPP