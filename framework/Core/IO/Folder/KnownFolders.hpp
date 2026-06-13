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

#ifndef ANKA_CORE_IO_KNOWNFOLDERS_HPP
#define ANKA_CORE_IO_KNOWNFOLDERS_HPP

#include <filesystem>

namespace anka::Core::IO::KnownFolders
{
   // The current user's standard directories, resolved through the Windows
   // shell (SHGetKnownFolderPath) so they follow redirection and roaming
   // instead of being guessed from the profile path. Each returns an empty
   // path only when the shell cannot resolve it, which callers read as "no
   // sensible default, fall back to whatever else fits". The Windows-API
   // detail stays in the implementation — the header leaks no shell types.
   std::filesystem::path desktop(void);

   // The current user's per-machine (non-roaming) application-data root,
   // %LOCALAPPDATA%. Empty when the shell cannot resolve it.
   std::filesystem::path localAppData(void);

   // A writable per-user data directory for the named application,
   // %LOCALAPPDATA%\<application>, created if it does not yet exist. This is
   // where databases and settings belong: the install directory (e.g. Program
   // Files) is read-only for standard users, so writing there fails. Empty when
   // the location cannot be resolved or the directory cannot be created.
   std::filesystem::path applicationData(const std::wstring& application);
}

#endif // ANKA_CORE_IO_KNOWNFOLDERS_HPP
