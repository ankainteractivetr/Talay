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

#ifndef ANKA_CORE_ERROR_EXCEPTION_OEXCEPTION_HPP
#define ANKA_CORE_ERROR_EXCEPTION_OEXCEPTION_HPP

#include "IException.hpp"

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace anka
{
	namespace Core
	{
		namespace Error
		{
			namespace Exception
			{
				/*
				*
				* Concerete Exception Class
				*
				*/
				class OException final : public IException
				{
					public :


						//
						// COTORs and DOTOR
						//

						OException();
						OException(const std::wstring& objectName,
									  const std::wstring& message,
									  const std::function<void()>& recoveryFunction);
						OException(const OException& exception) = delete;
						OException(OException&& exception) noexcept = delete;

						~OException() noexcept override;

						//
						// Operators
						//

						OException& operator=(const OException& object) = delete;
						OException& operator=(OException&& object) noexcept = delete;

						//
						// IObject overrides
						//

						const std::wstring& getName(void) const override;
						void                setName(const std::wstring& name) override;

						std::uint64_t getId(void) const override;

						//
						// IException overrides
						//

						const std::wstring& getErrorMessage(void) const override;
						void                setErrorMessage(const std::wstring& message) override;

						const std::function<void()>& getRecoveryFunction(void) const override;
						void                         setRecoveryFunction(const std::function<void()>& function) override;


					private:


						struct Impl;
						std::unique_ptr<Impl> m_pImpl;
				};
			}
		}
	}
}

#endif // ANKA_CORE_ERROR_EXCEPTION_OEXCEPTION_HPP
