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
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "OLocalizationService.hpp"

// Resource-system + globalization projections. Included after pch's XAML headers,
// as the Windows App SDK resource header requires.
#include <winrt/Microsoft.Windows.ApplicationModel.Resources.h>
#include <winrt/Microsoft.Windows.Globalization.h>

using namespace anka::Core::Object;
using namespace anka::Core::Localization;

namespace WinResources    = winrt::Microsoft::Windows::ApplicationModel::Resources;
namespace WinGlobalization = winrt::Microsoft::Windows::Globalization;

// -----------------------------------------------------------------------------
// Helpers — the resource id a code key maps to. The default Resources.resw lives
// under the "Resources" subtree of the main map, so every lookup is prefixed.
// -----------------------------------------------------------------------------
namespace
{
   winrt::hstring resourceUri(std::wstring_view key)
   {
      std::wstring uri {L"Resources/"};
      uri.append(key);
      return winrt::hstring {uri};
   }

   // Force the resource system (both x:Uid and ResourceManager) onto one language,
   // overriding the OS display language. Supported for unpackaged Windows App SDK
   // apps via the Microsoft.Windows.Globalization API; must run before XAML loads.
   void forceLanguage(const std::wstring& languageTag)
   {
      WinGlobalization::ApplicationLanguages::PrimaryLanguageOverride(winrt::hstring {languageTag});
   }
}

// -----------------------------------------------------------------------------
// OLocalizationService::Impl Definition
// -----------------------------------------------------------------------------
struct OLocalizationService::Impl
{
   ObjectIdentity m_identity {L"LocalizationServiceObject"};

   WinResources::ResourceMap     m_map {nullptr};
   WinResources::ResourceContext m_context {nullptr};
   bool                          m_ready {false};

   const std::wstring& getObjectName(void) const
   {
      return m_identity.name();
   }

   void setObjectName(const std::wstring& name)
   {
      m_identity.setName(name);
   }

   std::uint64_t getObjectId(void) const
   {
      return m_identity.id();
   }

   // Open the resource map once, forcing the language first when the host asked
   // for one. With no tag the context defaults to the system display language,
   // which — for an unpackaged app — is exactly what the x:Uid layer resolves
   // against, so both agree.
   void initialize(std::optional<std::wstring> languageTag)
   {
      try {
         if (languageTag) forceLanguage(*languageTag);   // build the context AFTER, so it inherits the override
         WinResources::ResourceManager manager {};
         m_map     = manager.MainResourceMap();
         m_context = manager.CreateResourceContext();
         m_ready   = true;
      } catch (...) {
         m_ready = false;
      }
   }

   // The current-language string for a key, or the key itself if anything is
   // missing (uninitialised service, absent resource) so gaps stay visible.
   std::wstring text(std::wstring_view key) const
   {
      if (!m_ready)
         return std::wstring {key};

      try {
         return std::wstring {m_map.GetValue(resourceUri(key), m_context).ValueAsString()};
      } catch (...) {
         return std::wstring {key};
      }
   }
};

// -----------------------------------------------------------------------------
// OLocalizationService Implementation
// -----------------------------------------------------------------------------
OLocalizationService::OLocalizationService() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OLocalizationService::~OLocalizationService() noexcept = default;
// -----------------------------------------------------------------------------
OLocalizationService& OLocalizationService::getInstance()
{
   static OLocalizationService s_instance;
   return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OLocalizationService::getName(void) const
{
   return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OLocalizationService::setName(const std::wstring& name)
{
   m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OLocalizationService::getId(void) const
{
   return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
void OLocalizationService::initialize(std::optional<std::wstring> languageTag)
{
   m_pImpl->initialize(std::move(languageTag));
}
// -----------------------------------------------------------------------------
std::wstring OLocalizationService::text(std::wstring_view key) const
{
   return m_pImpl->text(key);
}
// -----------------------------------------------------------------------------
