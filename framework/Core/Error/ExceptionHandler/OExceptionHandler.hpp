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

#ifndef ANKA_CORE_ERROR_EXCEPTIONHANDLER_OEXCEPTIONHANDLER_HPP
#define ANKA_CORE_ERROR_EXCEPTIONHANDLER_OEXCEPTIONHANDLER_HPP

#include "IExceptionHandler.hpp"

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
				class OExceptionHandler final : public IExceptionHandler
				{
					public:

						// Singleton
						static OExceptionHandler& getInstance();

						OExceptionHandler(const OExceptionHandler& handler) = delete;
						OExceptionHandler(OExceptionHandler&& handler) noexcept = delete;

						~OExceptionHandler() noexcept override;

						OExceptionHandler& operator=(const OExceptionHandler& handler) = delete;
						OExceptionHandler& operator=(OExceptionHandler&& handler) noexcept = delete;

						//
						// IObject overrides
						//

						const std::wstring& getName(void) const override;
						void                setName(const std::wstring& name) override;

						std::uint64_t getId(void) const override;
				
						//
						// IExceptionHandler overrides
						//

						bool handleException(const std::exception& exception) const override;
						bool handleException(const std::shared_ptr<anka::Core::Error::Exception::IException> exception) const override;

					private:

					   // Singleton COTOR
						OExceptionHandler();

						struct Impl;
						std::unique_ptr<Impl> m_pImpl;
				};
			}
		}
	}
}

#endif // ANKA_CORE_ERROR_EXCEPTIONHANDLER_OEXCEPTIONHANDLER_HPP