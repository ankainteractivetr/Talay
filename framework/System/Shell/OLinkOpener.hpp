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

#ifndef ANKA_SYSTEM_SHELL_OLINKOPENER_HPP
#define ANKA_SYSTEM_SHELL_OLINKOPENER_HPP

#include "ILinkOpener.hpp"

#include <memory>

namespace anka::System::Shell
{
   /*
   *
   * Concrete OLinkOpener Class (Singleton)
   *
   * Pure Windows-API implementation: ShellExecuteW with the "open" verb, which
   * defers browser choice to the user's default-apps settings. Stateless, so a
   * single instance per process matches the other framework shell services.
   *
   */
   class OLinkOpener final : public ILinkOpener
   {
      public:

         OLinkOpener(const OLinkOpener& object) = delete;
         OLinkOpener(OLinkOpener&& object) noexcept = delete;

         ~OLinkOpener() noexcept override;

         OLinkOpener& operator=(const OLinkOpener& object) = delete;
         OLinkOpener& operator=(OLinkOpener&& object) noexcept = delete;

         static OLinkOpener& getInstance();

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // ILinkOpener overrides
         //

         bool openWebLink(const std::wstring& url) override;

      private:

         OLinkOpener();

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_SYSTEM_SHELL_OLINKOPENER_HPP
