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
#include "OShellRegistrar.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/System/Windows/windows.hpp"

#include <shlobj.h>

#include <algorithm>
#include <cwctype>
#include <string>

#pragma comment(lib, "Advapi32.lib")
#pragma comment(lib, "Shell32.lib")

using namespace anka::Core::Object;
using namespace anka::System::Shell;

// -----------------------------------------------------------------------------
// Registry primitives -- the only place raw advapi32 calls live. Every helper
// is idempotent: writing overwrites, deleting something absent is a success.
// -----------------------------------------------------------------------------
namespace
{
   HKEY rootOf(RegistryScope scope)
   {
      return scope == RegistryScope::LocalMachine ? HKEY_LOCAL_MACHINE
                                                  : HKEY_CURRENT_USER;
   }

   // Extensions are normalised to lowercase: the registry compares without
   // case, but lowercase keys are the convention every tool expects.
   std::wstring lowered(std::wstring text)
   {
      std::transform(text.begin(), text.end(), text.begin(),
                     [](wchar_t ch) { return static_cast<wchar_t>(std::towlower(ch)); });
      return text;
   }

   // Create-or-open 'keyPath' and set one string value; nullptr value name
   // addresses the key's default value.
   bool writeValue(HKEY root, const std::wstring& keyPath,
                   const wchar_t* valueName, const std::wstring& data)
   {
      HKEY key {nullptr};
      if (RegCreateKeyExW(root, keyPath.c_str(), 0, nullptr, 0, KEY_SET_VALUE,
                          nullptr, &key, nullptr) != ERROR_SUCCESS)
         return false;

      const auto bytes {static_cast<DWORD>((data.size() + 1) * sizeof(wchar_t))};
      const LSTATUS status {RegSetValueExW(key, valueName, 0, REG_SZ,
                                           reinterpret_cast<const BYTE*>(data.c_str()), bytes)};
      RegCloseKey(key);
      return status == ERROR_SUCCESS;
   }

   bool deleteTree(HKEY root, const std::wstring& keyPath)
   {
      const LSTATUS status {RegDeleteTreeW(root, keyPath.c_str())};
      return status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND;
   }

   bool deleteValue(HKEY root, const std::wstring& keyPath, const std::wstring& valueName)
   {
      HKEY key {nullptr};
      if (RegOpenKeyExW(root, keyPath.c_str(), 0, KEY_SET_VALUE, &key) != ERROR_SUCCESS)
         return true;

      const LSTATUS status {RegDeleteValueW(key, valueName.c_str())};
      RegCloseKey(key);
      return status == ERROR_SUCCESS || status == ERROR_FILE_NOT_FOUND;
   }

   bool keyExists(HKEY root, const std::wstring& keyPath)
   {
      HKEY key {nullptr};
      if (RegOpenKeyExW(root, keyPath.c_str(), 0, KEY_READ, &key) != ERROR_SUCCESS)
         return false;

      RegCloseKey(key);
      return true;
   }

   // One string value, or empty when absent. The fixed buffer comfortably
   // holds any command line this registrar itself writes.
   std::wstring readValue(HKEY root, const std::wstring& keyPath, const wchar_t* valueName)
   {
      wchar_t buffer[1024] {};
      DWORD bytes {sizeof(buffer) - sizeof(wchar_t)};
      if (RegGetValueW(root, keyPath.c_str(), valueName, RRF_RT_REG_SZ, nullptr, buffer, &bytes) != ERROR_SUCCESS)
         return {};

      return buffer;
   }

   // Tell Explorer the association landscape changed, so menus and icons
   // refresh without a logoff.
   void notifyShell(void)
   {
      SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
   }

   //
   // Key-path builders -- the registry layout in one place.
   //

   std::wstring classesPath(const std::wstring& tail)
   {
      return L"Software\\Classes\\" + tail;
   }

   std::wstring capabilitiesPath(const std::wstring& applicationName)
   {
      return L"Software\\" + applicationName + L"\\Capabilities";
   }

   std::wstring verbPath(const std::wstring& extension, const std::wstring& verbId)
   {
      return classesPath(L"SystemFileAssociations\\" + lowered(extension) + L"\\shell\\" + verbId);
   }
}

// -----------------------------------------------------------------------------
// OShellRegistrar::Impl Definition
// -----------------------------------------------------------------------------
struct OShellRegistrar::Impl
{
   ObjectIdentity m_identity {L"ShellRegistrarObject"};

   // The ProgId: what "opening with this application" means.
   bool writeProgId(const AssociationSpec& spec) const
   {
      HKEY root {rootOf(spec.scope)};
      bool ok {writeValue(root, classesPath(spec.progId), nullptr, spec.friendlyName)};

      if (!spec.iconResource.empty())
         ok = writeValue(root, classesPath(spec.progId + L"\\DefaultIcon"), nullptr, spec.iconResource) && ok;

      return writeValue(root, classesPath(spec.progId + L"\\shell\\open\\command"), nullptr, spec.openCommand) && ok;
   }

   // Each extension lists the ProgId among its "Open with" candidates.
   bool writeOpenWithLists(const AssociationSpec& spec) const
   {
      bool ok {true};
      for (const std::wstring& extension : spec.extensions)
         ok = writeValue(rootOf(spec.scope),
                         classesPath(lowered(extension) + L"\\OpenWithProgids"),
                         spec.progId.c_str(), L"") && ok;

      return ok;
   }

   // The Capabilities entry puts the application on the Default Apps page.
   bool writeCapabilities(const AssociationSpec& spec) const
   {
      HKEY root {rootOf(spec.scope)};
      const std::wstring capabilities {capabilitiesPath(spec.applicationName)};

      bool ok {writeValue(root, capabilities, L"ApplicationName", spec.applicationName)};
      ok = writeValue(root, capabilities, L"ApplicationDescription", spec.applicationDescription) && ok;

      for (const std::wstring& extension : spec.extensions)
         ok = writeValue(root, capabilities + L"\\FileAssociations",
                         lowered(extension).c_str(), spec.progId) && ok;

      return ok;
   }

   bool writeRegisteredApplication(const AssociationSpec& spec) const
   {
      return writeValue(rootOf(spec.scope), L"Software\\RegisteredApplications",
                        spec.applicationName.c_str(), capabilitiesPath(spec.applicationName));
   }

   bool registerOpenWith(const AssociationSpec& spec) const
   {
      bool ok {writeProgId(spec)};
      ok = writeOpenWithLists(spec) && ok;
      ok = writeCapabilities(spec) && ok;
      ok = writeRegisteredApplication(spec) && ok;

      notifyShell();
      return ok;
   }

   bool removeOpenWithLists(const AssociationSpec& spec) const
   {
      bool ok {true};
      for (const std::wstring& extension : spec.extensions)
         ok = deleteValue(rootOf(spec.scope),
                          classesPath(lowered(extension) + L"\\OpenWithProgids"),
                          spec.progId) && ok;

      return ok;
   }

   // The inverse of registerOpenWith. Only the Capabilities subtree is removed,
   // not Software\<app> itself -- the application may keep other state there.
   bool unregisterOpenWith(const AssociationSpec& spec) const
   {
      HKEY root {rootOf(spec.scope)};
      bool ok {deleteTree(root, classesPath(spec.progId))};

      ok = removeOpenWithLists(spec) && ok;
      ok = deleteTree(root, capabilitiesPath(spec.applicationName)) && ok;
      ok = deleteValue(root, L"Software\\RegisteredApplications", spec.applicationName) && ok;

      notifyShell();
      return ok;
   }

   // The ProgId key is the one artefact every registration writes, so its
   // presence is the truth about "are we registered".
   bool isOpenWithRegistered(const AssociationSpec& spec) const
   {
      return keyExists(rootOf(spec.scope), classesPath(spec.progId));
   }

   // MultiSelectModel = Player lifts Explorer's default 15-item cap, so large
   // multi-selections still invoke the verb for every file.
   bool writeVerbFor(const std::wstring& extension, const VerbSpec& spec) const
   {
      HKEY root {rootOf(spec.scope)};
      const std::wstring base {verbPath(extension, spec.verbId)};

      bool ok {writeValue(root, base, nullptr, spec.caption)};
      ok = writeValue(root, base, L"MultiSelectModel", L"Player") && ok;

      if (!spec.iconResource.empty())
         ok = writeValue(root, base, L"Icon", spec.iconResource) && ok;

      return writeValue(root, base + L"\\command", nullptr, spec.command) && ok;
   }

   bool registerVerb(const VerbSpec& spec) const
   {
      bool ok {true};
      for (const std::wstring& extension : spec.extensions)
         ok = writeVerbFor(extension, spec) && ok;

      notifyShell();
      return ok;
   }

   bool unregisterVerb(const VerbSpec& spec) const
   {
      bool ok {true};
      for (const std::wstring& extension : spec.extensions)
         ok = deleteTree(rootOf(spec.scope), verbPath(extension, spec.verbId)) && ok;

      notifyShell();
      return ok;
   }

   // The first extension's command stands for the whole registration: every
   // extension is written in one go, so they never diverge.
   bool isVerbRegistered(const VerbSpec& spec) const
   {
      if (spec.extensions.empty())
         return false;

      const std::wstring base {verbPath(spec.extensions.front(), spec.verbId)};
      return readValue(rootOf(spec.scope), base + L"\\command", nullptr) == spec.command;
   }
};

// -----------------------------------------------------------------------------
// OShellRegistrar Implementation
// -----------------------------------------------------------------------------
OShellRegistrar::OShellRegistrar() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OShellRegistrar::~OShellRegistrar() noexcept = default;
// -----------------------------------------------------------------------------
OShellRegistrar& OShellRegistrar::getInstance()
{
   static OShellRegistrar s_instance;
   return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OShellRegistrar::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OShellRegistrar::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OShellRegistrar::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OShellRegistrar::registerOpenWith(const AssociationSpec& spec)
{
   return m_pImpl->registerOpenWith(spec);
}
// -----------------------------------------------------------------------------
bool OShellRegistrar::unregisterOpenWith(const AssociationSpec& spec)
{
   return m_pImpl->unregisterOpenWith(spec);
}
// -----------------------------------------------------------------------------
bool OShellRegistrar::isOpenWithRegistered(const AssociationSpec& spec) const
{
   return m_pImpl->isOpenWithRegistered(spec);
}
// -----------------------------------------------------------------------------
bool OShellRegistrar::registerVerb(const VerbSpec& spec)
{
   return m_pImpl->registerVerb(spec);
}
// -----------------------------------------------------------------------------
bool OShellRegistrar::unregisterVerb(const VerbSpec& spec)
{
   return m_pImpl->unregisterVerb(spec);
}
// -----------------------------------------------------------------------------
bool OShellRegistrar::isVerbRegistered(const VerbSpec& spec) const
{
   return m_pImpl->isVerbRegistered(spec);
}
// -----------------------------------------------------------------------------
