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

#ifndef ANKA_SYSTEM_SHELL_ILINKOPENER_HPP
#define ANKA_SYSTEM_SHELL_ILINKOPENER_HPP

#include "framework/Core/Object/IObject.hpp"

#include <string>

namespace anka::System::Shell
{
   /*
   * Abstract Link Opener Interface
   *
   * Hands a web address to whatever browser the user made the system default.
   * Callers say WHERE to go; how Windows resolves and launches the handler
   * stays an implementation detail. Throw-free: failure (no handler, refused
   * address) is reported through the result.
   *
   */
   class ILinkOpener : public anka::Core::Object::IObject
   {
      public:

         virtual ~ILinkOpener() noexcept override = default;

         // True when the system accepted the address. Only http(s) links are
         // forwarded, so a stray string can never start an arbitrary program.
         virtual bool openWebLink(const std::wstring& url) = 0;
   };
}

#endif // ANKA_SYSTEM_SHELL_ILINKOPENER_HPP
