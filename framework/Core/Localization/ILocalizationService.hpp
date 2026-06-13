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

#ifndef ANKA_CORE_LOCALIZATION_ILOCALIZATIONSERVICE_HPP
#define ANKA_CORE_LOCALIZATION_ILOCALIZATIONSERVICE_HPP

#include <optional>
#include <string>
#include <string_view>

#include "framework/Core/Object/IObject.hpp"

namespace anka::Core::Localization
{
   /*
   * Abstract Localization Service Interface (Facade)
   *
   * The single seam every code path uses to read a localized string. XAML is
   * localized by the platform through x:Uid; this facade covers the strings that
   * are built in code (dialog text, button labels) and owns the language the app
   * resolved at start-up. One implementation reads the same Resources.resw the
   * XAML layer does, so there is exactly one source of truth behind both.
   *
   */
   class ILocalizationService : public anka::Core::Object::IObject
   {
      public:

         virtual ~ILocalizationService() noexcept override = default;

         // Stand up the resource reader, optionally forcing a language. Call once,
         // before any UI is built. A set 'languageTag' (a BCP-47 tag such as
         // L"tr-TR") is forced onto the resource system so x:Uid honours it; an
         // empty one falls back to the OS display language. The forcing only takes
         // on the x:Uid layer when applied before the first XAML loads. The host
         // supplies the tag, so this service stays language-set agnostic.
         virtual void initialize(std::optional<std::wstring> languageTag) = 0;

         // The current-language text for a resource id (e.g. L"CommonOk"). A
         // missing id yields the id itself, so gaps surface in the UI instead of
         // crashing or showing blanks.
         virtual std::wstring text(std::wstring_view key) const = 0;
   };
}

#endif // ANKA_CORE_LOCALIZATION_ILOCALIZATIONSERVICE_HPP
