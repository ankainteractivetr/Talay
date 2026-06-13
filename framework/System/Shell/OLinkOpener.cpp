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
#include "OLinkOpener.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/System/Windows/windows.hpp"

#include <shellapi.h>

#pragma comment(lib, "Shell32.lib")

using namespace anka::Core::Object;
using namespace anka::System::Shell;

namespace
{
   // The whitelist that keeps openWebLink a browser call and nothing more:
   // anything not explicitly http(s) is refused before it reaches the shell.
   bool isWebAddress(const std::wstring& url)
   {
      return url.starts_with(L"http://") || url.starts_with(L"https://");
   }

   // ShellExecuteW reports success as a pseudo-handle value above 32; the
   // verb nullptr means "the default action", i.e. the user's default browser.
   bool launchViaShell(const std::wstring& url)
   {
      const HINSTANCE result {ShellExecuteW(nullptr, nullptr, url.c_str(),
                                            nullptr, nullptr, SW_SHOWNORMAL)};
      return reinterpret_cast<INT_PTR>(result) > 32;
   }
}

// -----------------------------------------------------------------------------
struct OLinkOpener::Impl
{
   ObjectIdentity m_identity {L"LinkOpenerObject"};
};
// -----------------------------------------------------------------------------
OLinkOpener::OLinkOpener() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OLinkOpener::~OLinkOpener() noexcept = default;
// -----------------------------------------------------------------------------
OLinkOpener& OLinkOpener::getInstance()
{
   static OLinkOpener s_instance;
   return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OLinkOpener::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OLinkOpener::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OLinkOpener::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OLinkOpener::openWebLink(const std::wstring& url)
{
   if (!isWebAddress(url))
      return false;

   return launchViaShell(url);
}
