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

#ifndef TALAY_SHELL_TALAYSHELL_HPP
#define TALAY_SHELL_TALAYSHELL_HPP

#include "framework/System/Shell/IShellRegistrar.hpp"

namespace talay::Shell
{
   // Talay's identity for the generic shell registrar, built fresh each call
   // so the executable path and the localized captions are always current.
   //
   // makeOpenWithSpec  : "Open with Talay" + Default Apps listing, per user
   //                     (HKCU) -- driven by the settings toggle.
   // makeConvertVerbSpec: the "Convert with Talay" context-menu verb, also
   //                     per user (HKCU) -- ensured at every normal start
   //                     and by the installer hook; no elevation needed.
   anka::System::Shell::AssociationSpec makeOpenWithSpec(void);

   anka::System::Shell::VerbSpec makeConvertVerbSpec(void);
}

#endif // TALAY_SHELL_TALAYSHELL_HPP
