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

#ifndef TALAY_LOCALIZATION_ILANGUAGEDETECTOR_HPP
#define TALAY_LOCALIZATION_ILANGUAGEDETECTOR_HPP

#include "Language.hpp"

namespace talay::Localization
{
   /*
   * Abstract Language Detector Interface (Strategy)
   *
   * Decides which Language the application should start in. Kept deliberately
   * minimal — a stateless, identity-less strategy (ISP) — so the platform query
   * behind it can be swapped (e.g. for a test) without dragging an object model
   * along. The localization service depends on this interface, never on the
   * Win32 call behind it (DIP).
   *
   */
   class ILanguageDetector
   {
      public:

         virtual ~ILanguageDetector() noexcept = default;

         // The language Talay should present, derived from the environment.
         virtual Language detect(void) const = 0;
   };
}

#endif // TALAY_LOCALIZATION_ILANGUAGEDETECTOR_HPP
