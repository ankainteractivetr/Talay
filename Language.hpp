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

#ifndef TALAY_LOCALIZATION_LANGUAGE_HPP
#define TALAY_LOCALIZATION_LANGUAGE_HPP

#include <string_view>

namespace talay::Localization
{
   // The languages Talay ships. English is the default and the fallback for any
   // system that is not Turkish, so it is listed first. A closed set on purpose:
   // adding a language is a deliberate edit, not a configuration accident.
   enum class Language
   {
      English,
      Turkish
   };

   // The BCP-47 tag that names a language's resource folder (Strings/<tag>/) and
   // the qualifier the resource system matches against.
   constexpr std::wstring_view toBcp47(Language language) noexcept
   {
      return language == Language::Turkish ? std::wstring_view {L"tr-TR"}
                                           : std::wstring_view {L"en-US"};
   }

   // The inverse of toBcp47. Only Turkish is recognised explicitly; everything
   // else — including a malformed or unknown tag — resolves to English.
   constexpr Language fromBcp47(std::wstring_view tag) noexcept
   {
      return tag.starts_with(L"tr") ? Language::Turkish : Language::English;
   }
}

#endif // TALAY_LOCALIZATION_LANGUAGE_HPP
