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

#ifndef TALAY_SHELL_ISHELLINTEGRATION_HPP
#define TALAY_SHELL_ISHELLINTEGRATION_HPP

#include "framework/Core/Object/IObject.hpp"

namespace talay::Shell
{
   /*
   * Abstract Shell Integration Interface
   *
   * The lifecycle of Talay's Windows shell footprint gathered behind one
   * facade, so every caller speaks intent ("install", "uninstall") and no
   * caller re-derives the registry specs. Two footprints with different
   * policies live here:
   *
   *   - the "Convert with Talay" context-menu verb -- a core feature, always
   *     present and self-healing toward THIS executable;
   *   - the "Open with" / Default-Apps file-type associations -- on by default
   *     but user-toggleable, so they are registered by the installer and
   *     reconciled by the settings dialog.
   *
   * A thin coordinator over the framework's application-agnostic registrar:
   * it owns only WHICH specs and WHEN; the registrar owns the registry layout.
   * Every call is best-effort and throw-free.
   *
   */
   class IShellIntegration : public anka::Core::Object::IObject
   {
      public:

         virtual ~IShellIntegration() noexcept override = default;

         // First install (setup --register-shell): list every supported image
         // type under "Open with" AND add the convert verb.
         virtual void install(void) = 0;

         // Uninstall (setup --unregister-shell): remove the verb (current-user
         // and the machine-wide one older builds wrote) and the file-type
         // associations, leaving the shell as Talay found it.
         virtual void uninstall(void) = 0;

         // Normal start: cheaply re-assert the convert verb so it always points
         // at THIS executable (a moved or dev build self-heals); a no-op when
         // the verb is already current.
         virtual void ensureVerb(void) = 0;

         // Reconcile the file-type associations to 'registered' -- the settings
         // toggle's effect, applied only when the state actually changes.
         virtual void applyFileTypes(bool registered) = 0;
   };
}

#endif // TALAY_SHELL_ISHELLINTEGRATION_HPP
