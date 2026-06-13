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

#ifndef ANKA_SYSTEM_SHELL_ISHELLREGISTRAR_HPP
#define ANKA_SYSTEM_SHELL_ISHELLREGISTRAR_HPP

#include "framework/Core/Object/IObject.hpp"

#include <string>
#include <vector>

namespace anka::System::Shell
{
   // Which registry hive an operation writes. CurrentUser needs no elevation
   // and affects only the logged-in user; LocalMachine is for elevated
   // installers that register for every user.
   enum class RegistryScope
   {
      CurrentUser,
      LocalMachine
   };

   // Everything Windows needs to list an application under "Open with" and on
   // the Default Apps page: a ProgId describing how to open a file, the
   // extensions that point at it, and the Capabilities entry that names the
   // application itself. Aggregates on purpose -- call sites read as C++20
   // designated initialisers and the registrar stays application-agnostic.
   struct AssociationSpec
   {
      std::wstring applicationName;          // RegisteredApplications entry, e.g. L"Talay"
      std::wstring applicationDescription;
      std::wstring progId;                   // e.g. L"Talay.Image"
      std::wstring friendlyName;             // what Explorer shows for the ProgId
      std::wstring openCommand;              // e.g. L"\"C:\\...\\app.exe\" \"%1\""
      std::wstring iconResource;             // e.g. L"C:\\...\\app.exe,0"
      std::vector<std::wstring> extensions;  // dotted, any case
      RegistryScope scope {RegistryScope::CurrentUser};
   };

   // A context-menu entry under SystemFileAssociations: one verb shown for
   // every listed extension, invoking 'command' with the clicked file as %1.
   struct VerbSpec
   {
      std::wstring verbId;                   // registry key name, e.g. L"Talay.Convert"
      std::wstring caption;                  // menu text
      std::wstring command;                  // e.g. L"\"C:\\...\\app.exe\" --convert \"%1\""
      std::wstring iconResource;
      std::vector<std::wstring> extensions;  // dotted, any case
      RegistryScope scope {RegistryScope::LocalMachine};
   };

   /*
   * Abstract Shell Registrar Interface
   *
   * Facade over the raw registry layout Windows expects for file-type and
   * context-menu integration. Callers describe WHAT to register through the
   * spec aggregates; where the keys live and how the shell is told about the
   * change stay implementation details. Every call is throw-free and reports
   * failure (e.g. writing LocalMachine without elevation) through its result.
   *
   * Note: since Windows 10 an application cannot make itself the DEFAULT
   * handler programmatically -- registering only makes it available; the user
   * confirms via "Open with" or the Default Apps page.
   *
   */
   class IShellRegistrar : public anka::Core::Object::IObject
   {
      public:

         virtual ~IShellRegistrar() noexcept override = default;

         //
         // "Open with" / Default Apps association
         //

         virtual bool registerOpenWith(const AssociationSpec& spec) = 0;

         virtual bool unregisterOpenWith(const AssociationSpec& spec) = 0;

         virtual bool isOpenWithRegistered(const AssociationSpec& spec) const = 0;

         //
         // Context-menu verb
         //

         virtual bool registerVerb(const VerbSpec& spec) = 0;

         virtual bool unregisterVerb(const VerbSpec& spec) = 0;

         // True when the verb already launches exactly this spec's command —
         // false for an absent verb AND for one pointing at a moved/old
         // executable, so callers can re-register only when needed.
         virtual bool isVerbRegistered(const VerbSpec& spec) const = 0;
   };
}

#endif // ANKA_SYSTEM_SHELL_ISHELLREGISTRAR_HPP
