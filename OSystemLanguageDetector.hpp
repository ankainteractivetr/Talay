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

#ifndef TALAY_LOCALIZATION_OSYSTEMLANGUAGEDETECTOR_HPP
#define TALAY_LOCALIZATION_OSYSTEMLANGUAGEDETECTOR_HPP

#include "ILanguageDetector.hpp"

namespace talay::Localization
{
   /*
   *
   * Concrete System Language Detector Class
   *
   * Reads the Windows display (UI) language and maps it to a Language: Turkish
   * when the primary language is Turkish, English otherwise. This is the same
   * "system display language" the unpackaged resource resolver uses to pick a
   * Resources.resw, so what this reports always matches what the UI shows.
   *
   * Stateless, so no pimpl is needed; the <Windows.h> dependency stays in the
   * .cpp and never leaks through this header.
   *
   */
   class OSystemLanguageDetector final : public ILanguageDetector
   {
      public:

         OSystemLanguageDetector() = default;

         ~OSystemLanguageDetector() noexcept override = default;

         //
         // ILanguageDetector overrides
         //

         Language detect(void) const override;
   };
}

#endif // TALAY_LOCALIZATION_OSYSTEMLANGUAGEDETECTOR_HPP
