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

#ifndef ANKA_CORE_LOCALIZATION_OLOCALIZATIONSERVICE_HPP
#define ANKA_CORE_LOCALIZATION_OLOCALIZATIONSERVICE_HPP

#include "ILocalizationService.hpp"

#include <memory>

namespace anka::Core::Localization
{
   /*
   *
   * Concrete Localization Service Class
   *
   * A facade over the Windows App SDK resource system. Single instance per
   * process (one resource map, one display language), matching the other
   * framework services. All resource-system state is hidden behind a pimpl so
   * this header stays free of WinRT.
   *
   */
   class OLocalizationService final : public ILocalizationService
   {
      public:

         // Singleton
         static OLocalizationService& getInstance();

         OLocalizationService(const OLocalizationService& service) = delete;
         OLocalizationService(OLocalizationService&& service) noexcept = delete;

         ~OLocalizationService() noexcept override;

         OLocalizationService& operator=(const OLocalizationService& service) = delete;
         OLocalizationService& operator=(OLocalizationService&& service) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // ILocalizationService overrides
         //

         void initialize(std::optional<std::wstring> languageTag) override;

         std::wstring text(std::wstring_view key) const override;

      private:

         // Singleton COTOR
         OLocalizationService();

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_CORE_LOCALIZATION_OLOCALIZATIONSERVICE_HPP
