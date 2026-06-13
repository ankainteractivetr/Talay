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

#ifndef ANKA_SYSTEM_PROCESS_EXECUTABLEPATH_HPP
#define ANKA_SYSTEM_PROCESS_EXECUTABLEPATH_HPP

#include <filesystem>

namespace anka::System::Process
{
   // The full path to the running executable, asked of the OS each call
   // (GetModuleFileNameW) rather than guessed from argv[0] — so a build that
   // was moved, or launched through a symlink, still resolves to where it
   // actually runs. The Win32 detail stays in the implementation; the header
   // leaks no Windows types.
   std::filesystem::path executablePath(void);

   // The directory that executable sits in: where an unpackaged app keeps the
   // files it ships beside itself and writes its own (the settings ini, the
   // library database).
   std::filesystem::path executableDirectory(void);
}

#endif // ANKA_SYSTEM_PROCESS_EXECUTABLEPATH_HPP
