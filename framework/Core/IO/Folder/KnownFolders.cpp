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
#include "KnownFolders.hpp"

#include <shlobj.h>

#include <memory>
#include <system_error>

#pragma comment(lib, "Shell32.lib")

namespace
{
   // SHGetKnownFolderPath allocates the string with CoTaskMemAlloc; a unique_ptr
   // with the matching deleter frees it however the function returns.
   using CoString = std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)>;

   std::filesystem::path resolve(REFKNOWNFOLDERID folderId)
   {
      PWSTR raw {nullptr};
      if (FAILED(SHGetKnownFolderPath(folderId, KF_FLAG_DEFAULT, nullptr, &raw)))
         return {};

      const CoString owned {raw, &CoTaskMemFree};
      return std::filesystem::path {owned.get()};
   }
}

// -----------------------------------------------------------------------------
std::filesystem::path anka::Core::IO::KnownFolders::desktop(void)
{
   return resolve(FOLDERID_Desktop);
}
// -----------------------------------------------------------------------------
std::filesystem::path anka::Core::IO::KnownFolders::localAppData(void)
{
   return resolve(FOLDERID_LocalAppData);
}
// -----------------------------------------------------------------------------
std::filesystem::path anka::Core::IO::KnownFolders::applicationData(
   const std::wstring& application)
{
   const auto root = localAppData();
   if (root.empty())
      return {};

   const auto folder = root / application;
   std::error_code ec;
   std::filesystem::create_directories(folder, ec);
   return ec ? std::filesystem::path {} : folder;
}
// -----------------------------------------------------------------------------
