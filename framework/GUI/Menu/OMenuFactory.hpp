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

#ifndef ANKA_GUI_MENU_OMENUFACTORY_HPP
#define ANKA_GUI_MENU_OMENUFACTORY_HPP

#include "IMenuFactory.hpp"

#include <memory>

namespace anka::GUI::Menu
{
   /*
   *
   * Concrete Menu Factory Class
   *
   * Translates MenuCommand descriptors into themed MenuFlyout trees: one
   * flyout item per command with the command's action injected as the
   * item's Click behaviour, while Separator descriptors become plain
   * MenuFlyoutSeparator dividers. Stateless by nature, so a single instance
   * per process suffices, matching the other framework UI services. WinUI
   * construction details stay hidden behind a pimpl.
   *
   */
   class OMenuFactory final : public IMenuFactory
   {
      public:

         // Singleton
         static OMenuFactory& getInstance();

         OMenuFactory(const OMenuFactory& factory) = delete;
         OMenuFactory(OMenuFactory&& factory) noexcept = delete;

         ~OMenuFactory() noexcept override;

         OMenuFactory& operator=(const OMenuFactory& factory) = delete;
         OMenuFactory& operator=(OMenuFactory&& factory) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IMenuFactory overrides
         //

         winrt::Microsoft::UI::Xaml::Controls::MenuFlyout
            createFlyout(std::span<const MenuCommand> commands) const override;

         void attachContextMenu(const winrt::Microsoft::UI::Xaml::UIElement& target,
                                std::span<const MenuCommand> commands) const override;

      private:

         // Singleton COTOR
         OMenuFactory();

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_MENU_OMENUFACTORY_HPP
