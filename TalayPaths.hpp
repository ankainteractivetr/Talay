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

#ifndef TALAY_PATHS_TALAYPATHS_HPP
#define TALAY_PATHS_TALAYPATHS_HPP

#include <filesystem>

namespace talay::Paths
{
   // The one writable directory every user-data file shares: the database
   // (Talay.library.db) and settings.ini. It is %LOCALAPPDATA%\Talay, created
   // on first use, because the install directory (Program Files under the
   // installer) is read-only for standard users — writing there silently fails
   // and the library, recent files and saved settings all come up empty. Falls
   // back to the executable's own directory only when the shell cannot resolve
   // LocalAppData, so a usable path is always returned.
   std::filesystem::path userDataDirectory(void);
}

#endif // TALAY_PATHS_TALAYPATHS_HPP
