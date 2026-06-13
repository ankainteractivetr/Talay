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
#include "LaunchRequest.hpp"
#include "framework/System/Windows/windows.hpp"

#include <shellapi.h>

#include <sstream>

#pragma comment(lib, "Shell32.lib")

using namespace talay::Launch;

namespace
{
   // The process's arguments without the executable's own path.
   std::vector<std::wstring> argumentsOf(void)
   {
      int count {0};
      wchar_t** argv {CommandLineToArgvW(GetCommandLineW(), &count)};
      if (!argv)
         return {};

      std::vector<std::wstring> arguments;
      if (count > 1)
         arguments.assign(argv + 1, argv + count);

      LocalFree(argv);
      return arguments;
   }

   std::vector<std::wstring> splitLines(const std::wstring& text)
   {
      std::vector<std::wstring> lines;

      std::wistringstream stream {text};
      for (std::wstring line; std::getline(stream, line, L'\n');)
         if (!line.empty())
            lines.push_back(line);

      return lines;
   }
}

// -----------------------------------------------------------------------------
LaunchRequest talay::Launch::parseCommandLine(void)
{
   const std::vector<std::wstring> arguments {argumentsOf()};
   if (arguments.empty())
      return {};

   if (arguments.front() == L"--register-shell")   return {LaunchMode::RegisterShell, {}};
   if (arguments.front() == L"--unregister-shell") return {LaunchMode::UnregisterShell, {}};
   if (arguments.front() == k_relaunchArg)          return {LaunchMode::Relaunch, {}};
   if (arguments.front() == L"--convert")          return {LaunchMode::Convert, {arguments.begin() + 1, arguments.end()}};

   return {LaunchMode::OpenFile, {arguments.front()}};
}
// -----------------------------------------------------------------------------
std::wstring talay::Launch::encodePayload(const LaunchRequest& request)
{
   std::wstring payload {request.mode == LaunchMode::Convert ? L"convert" : L"open"};
   for (const std::wstring& file : request.files)
      payload += L'\n' + file;

   return payload;
}
// -----------------------------------------------------------------------------
LaunchRequest talay::Launch::decodePayload(const std::wstring& payload)
{
   const std::vector<std::wstring> lines {splitLines(payload)};
   if (lines.empty())
      return {};

   LaunchRequest request;
   request.mode  = lines.front() == L"convert" ? LaunchMode::Convert : LaunchMode::OpenFile;
   request.files = {lines.begin() + 1, lines.end()};
   return request;
}
// -----------------------------------------------------------------------------
