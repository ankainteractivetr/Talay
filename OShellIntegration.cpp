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
#include "OShellIntegration.hpp"
#include "TalayShell.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/System/Shell/IShellRegistrar.hpp"

using namespace talay::Shell;
using anka::Core::Object::ObjectIdentity;
using anka::System::Shell::AssociationSpec;
using anka::System::Shell::IShellRegistrar;
using anka::System::Shell::RegistryScope;
using anka::System::Shell::VerbSpec;

// -----------------------------------------------------------------------------
// OShellIntegration::Impl Definition
// -----------------------------------------------------------------------------
struct OShellIntegration::Impl
{
   ObjectIdentity m_identity {L"ShellIntegrationObject"};

   IShellRegistrar& m_registrar;

   explicit Impl(IShellRegistrar& registrar) :
      m_registrar {registrar}
   {}

   void install(void) const
   {
      m_registrar.registerOpenWith(makeOpenWithSpec());
      m_registrar.registerVerb(makeConvertVerbSpec());
   }

   void uninstall(void) const
   {
      removeVerbEverywhere();
      m_registrar.unregisterOpenWith(makeOpenWithSpec());
   }

   // Older builds wrote the verb machine-wide; clear both hives so an
   // upgrade-then-uninstall never strands a dead entry under HKLM.
   void removeVerbEverywhere(void) const
   {
      VerbSpec verb {makeConvertVerbSpec()};
      m_registrar.unregisterVerb(verb);

      verb.scope = RegistryScope::LocalMachine;
      m_registrar.unregisterVerb(verb);
   }

   void ensureVerb(void) const
   {
      const VerbSpec verb {makeConvertVerbSpec()};
      if (!m_registrar.isVerbRegistered(verb))
         m_registrar.registerVerb(verb);
   }

   // Only a real flip touches the registry -- re-registering on every save
   // would needlessly ripple SHChangeNotify through the shell.
   void applyFileTypes(bool registered) const
   {
      const AssociationSpec spec {makeOpenWithSpec()};
      if (registered == m_registrar.isOpenWithRegistered(spec))
         return;

      if (registered)
         m_registrar.registerOpenWith(spec);
      else
         m_registrar.unregisterOpenWith(spec);
   }
};

// -----------------------------------------------------------------------------
// OShellIntegration Implementation
// -----------------------------------------------------------------------------
OShellIntegration::OShellIntegration(IShellRegistrar& registrar) :
   m_pImpl {std::make_unique<Impl>(registrar)}
{}
// -----------------------------------------------------------------------------
OShellIntegration::~OShellIntegration() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OShellIntegration::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OShellIntegration::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OShellIntegration::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OShellIntegration::install(void)
{
   m_pImpl->install();
}
// -----------------------------------------------------------------------------
void OShellIntegration::uninstall(void)
{
   m_pImpl->uninstall();
}
// -----------------------------------------------------------------------------
void OShellIntegration::ensureVerb(void)
{
   m_pImpl->ensureVerb();
}
// -----------------------------------------------------------------------------
void OShellIntegration::applyFileTypes(bool registered)
{
   m_pImpl->applyFileTypes(registered);
}
// -----------------------------------------------------------------------------
