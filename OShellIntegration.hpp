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

#ifndef TALAY_SHELL_OSHELLINTEGRATION_HPP
#define TALAY_SHELL_OSHELLINTEGRATION_HPP

#include "IShellIntegration.hpp"

#include <memory>
#include <string>

namespace anka::System::Shell { class IShellRegistrar; }

namespace talay::Shell
{
   /*
   * Concrete Shell Integration Class
   *
   * Coordinates Talay's two registry footprints (the convert verb and the
   * file-type associations) onto an injected registrar -- the only collaborator
   * it needs, so the same instance serves the installer hooks, the startup
   * self-heal and the settings toggle. Holds the registrar by reference: the
   * caller owns its lifetime (the process-wide registrar singleton).
   *
   */
   class OShellIntegration final : public IShellIntegration
   {
      public:

         explicit OShellIntegration(anka::System::Shell::IShellRegistrar& registrar);

         OShellIntegration(const OShellIntegration& object) = delete;
         OShellIntegration(OShellIntegration&& object) noexcept = delete;

         ~OShellIntegration() noexcept override;

         OShellIntegration& operator=(const OShellIntegration& object) = delete;
         OShellIntegration& operator=(OShellIntegration&& object) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IShellIntegration overrides
         //

         void install(void) override;
         void uninstall(void) override;
         void ensureVerb(void) override;
         void applyFileTypes(bool registered) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_SHELL_OSHELLINTEGRATION_HPP
