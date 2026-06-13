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
#include "OInstanceChannel.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/System/Windows/windows.hpp"

#include <string>
#include <utility>

using namespace anka::Core::Object;
using namespace anka::System::Instance;

namespace
{
   // A send right after a failed claim may race the winner still creating its
   // window (Explorer spawns every selected file's process at once), so the
   // sender retries for up to two seconds before giving up.
   constexpr int   k_sendAttempts     {40};
   constexpr DWORD k_sendRetryDelayMs {50};
   constexpr UINT  k_sendTimeoutMs    {3000};

   // One window class per channel, derived from its name, so unrelated
   // channels can never receive each other's payloads.
   std::wstring makeClassName(const std::wstring& channelName)
   {
      return channelName + L".Channel";
   }
}

// -----------------------------------------------------------------------------
// OInstanceChannel::Impl Definition
// -----------------------------------------------------------------------------
struct OInstanceChannel::Impl
{
   ObjectIdentity m_identity {L"InstanceChannelObject"};

   HANDLE m_mutex  {nullptr};
   HWND   m_window {nullptr};

   ChannelMessageCallback m_onMessage;

   Impl() = default;

   explicit Impl(const std::wstring& objectName) :
      m_identity {objectName}
   {}

   ~Impl() noexcept
   {
      if (m_window)
         DestroyWindow(m_window);

      if (m_mutex)
         CloseHandle(m_mutex);
   }

   // The named mutex is the election: whoever creates it first owns the
   // channel; everyone else sees ERROR_ALREADY_EXISTS and stays a sender.
   bool claim(const std::wstring& channelName)
   {
      m_mutex = CreateMutexW(nullptr, TRUE, (L"Local\\" + channelName).c_str());
      if (!m_mutex)
         return false;

      if (GetLastError() == ERROR_ALREADY_EXISTS) {
         CloseHandle(m_mutex);
         m_mutex = nullptr;
         return false;
      }

      return createMessageWindow(channelName);
   }

   bool isOwner(void) const
   {
      return m_window != nullptr;
   }

   // Senders may or may not include a terminating null in cbData; trailing
   // nulls are trimmed so the payload arrives exactly as it was sent.
   void deliver(const COPYDATASTRUCT& data) const
   {
      if (!m_onMessage)
         return;

      const auto* text {static_cast<const wchar_t*>(data.lpData)};
      std::size_t length {data.cbData / sizeof(wchar_t)};
      while (length > 0 && text[length - 1] == L'\0')
         --length;

      m_onMessage(std::wstring {text, text + length});
   }

   static void rememberOwner(HWND window, LPARAM lParam)
   {
      const auto* create {reinterpret_cast<CREATESTRUCTW*>(lParam)};
      SetWindowLongPtrW(window, GWLP_USERDATA,
                        reinterpret_cast<LONG_PTR>(create->lpCreateParams));
   }

   static LRESULT deliverTo(HWND window, LPARAM lParam)
   {
      const auto* impl {reinterpret_cast<const Impl*>(GetWindowLongPtrW(window, GWLP_USERDATA))};
      if (impl)
         impl->deliver(*reinterpret_cast<const COPYDATASTRUCT*>(lParam));

      return TRUE;
   }

   static LRESULT CALLBACK channelProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
   {
      if (message == WM_NCCREATE)
         rememberOwner(window, lParam);

      if (message == WM_COPYDATA)
         return deliverTo(window, lParam);

      return DefWindowProcW(window, message, wParam, lParam);
   }

   static void registerWindowClass(const std::wstring& className)
   {
      WNDCLASSW windowClass {};
      windowClass.lpfnWndProc   = channelProc;
      windowClass.hInstance     = GetModuleHandleW(nullptr);
      windowClass.lpszClassName = className.c_str();

      RegisterClassW(&windowClass);   // already-registered is fine
   }

   // Message-only window (HWND_MESSAGE parent): invisible, costs nothing, and
   // receives WM_COPYDATA on the thread that created it.
   bool createMessageWindow(const std::wstring& channelName)
   {
      const std::wstring className {makeClassName(channelName)};
      registerWindowClass(className);

      m_window = CreateWindowExW(0, className.c_str(), channelName.c_str(), 0,
                                 0, 0, 0, 0, HWND_MESSAGE, nullptr,
                                 GetModuleHandleW(nullptr), this);
      return m_window != nullptr;
   }

   // Pass our foreground right to the owner: a freshly launched process has
   // it, and granting it lets the owner legitimately pull its window up in
   // response to this message instead of just flashing on the taskbar.
   static void grantForegroundTo(HWND owner)
   {
      DWORD processId {0};
      GetWindowThreadProcessId(owner, &processId);
      AllowSetForegroundWindow(processId);
   }

   static bool trySend(const std::wstring& channelName, const std::wstring& payload)
   {
      HWND owner {FindWindowExW(HWND_MESSAGE, nullptr,
                                makeClassName(channelName).c_str(), channelName.c_str())};
      if (!owner)
         return false;

      grantForegroundTo(owner);
      COPYDATASTRUCT data {0,
                           static_cast<DWORD>(payload.size() * sizeof(wchar_t)),
                           const_cast<wchar_t*>(payload.c_str())};

      DWORD_PTR result {};
      return SendMessageTimeoutW(owner, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&data),
                                 SMTO_BLOCK, k_sendTimeoutMs, &result) != 0;
   }

   static bool send(const std::wstring& channelName, const std::wstring& payload)
   {
      for (int attempt {0}; attempt < k_sendAttempts; ++attempt) {
         if (trySend(channelName, payload))
            return true;

         Sleep(k_sendRetryDelayMs);
      }

      return false;
   }
};

// -----------------------------------------------------------------------------
// OInstanceChannel Implementation
// -----------------------------------------------------------------------------
OInstanceChannel::OInstanceChannel() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OInstanceChannel::OInstanceChannel(const std::wstring& objectName) :
   m_pImpl {std::make_unique<Impl>(objectName)}
{}
// -----------------------------------------------------------------------------
OInstanceChannel::~OInstanceChannel() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OInstanceChannel::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OInstanceChannel::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OInstanceChannel::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OInstanceChannel::claim(const std::wstring& channelName)
{
   return m_pImpl->claim(channelName);
}
// -----------------------------------------------------------------------------
bool OInstanceChannel::isOwner(void) const
{
   return m_pImpl->isOwner();
}
// -----------------------------------------------------------------------------
void OInstanceChannel::onMessage(ChannelMessageCallback callback)
{
   m_pImpl->m_onMessage = std::move(callback);
}
// -----------------------------------------------------------------------------
bool OInstanceChannel::send(const std::wstring& channelName, const std::wstring& payload)
{
   return Impl::send(channelName, payload);
}
// -----------------------------------------------------------------------------
