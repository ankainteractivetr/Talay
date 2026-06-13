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
#include "TalayShell.hpp"
#include "ImageFormats.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"
#include "framework/System/Process/ExecutablePath.hpp"

#include <string>

using namespace anka::System::Shell;
using anka::Core::Localization::OLocalizationService;

namespace
{
   std::wstring quoted(const std::wstring& text)
   {
      return L"\"" + text + L"\"";
   }
}

// -----------------------------------------------------------------------------
AssociationSpec talay::Shell::makeOpenWithSpec(void)
{
   const std::wstring exe {anka::System::Process::executablePath().wstring()};

   return {.applicationName        = L"Talay",
           .applicationDescription = OLocalizationService::getInstance().text(L"ShellAppDescription"),
           .progId                 = L"Talay.Image",
           .friendlyName           = L"Talay Image",
           .openCommand            = quoted(exe) + L" " + quoted(L"%1"),
           .iconResource           = exe + L",0",
           .extensions             = talay::Formats::supportedImageFormats(),
           .scope                  = RegistryScope::CurrentUser};
}
// -----------------------------------------------------------------------------
// Per-user on purpose: HKCU needs no elevation, so dev builds and the
// installer's hook both succeed, and the app can self-heal the entry at
// startup whenever the executable moved.
VerbSpec talay::Shell::makeConvertVerbSpec(void)
{
   const std::wstring exe {anka::System::Process::executablePath().wstring()};

   return {.verbId       = L"Talay.Convert",
           .caption      = OLocalizationService::getInstance().text(L"ShellConvertVerb"),
           .command      = quoted(exe) + L" --convert " + quoted(L"%1"),
           .iconResource = exe + L",0",
           .extensions   = talay::Formats::supportedImageFormats(),
           .scope        = RegistryScope::CurrentUser};
}
// -----------------------------------------------------------------------------
