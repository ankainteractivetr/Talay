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

#ifndef ANKA_GUI_CURSOR_OHOVERCURSORCONTROLLER_HPP
#define ANKA_GUI_CURSOR_OHOVERCURSORCONTROLLER_HPP

#include "IHoverCursorController.hpp"
#include "ICursorRule.hpp"

#include <winrt/Microsoft.UI.Xaml.h>

#include <functional>
#include <memory>
#include <vector>

namespace anka::GUI::Cursor
{
   // Where a resolved shape is shown — the surface that owns the cursor for
   // the host's subtree. Injected (DIP) so the controller never names a
   // concrete control.
   using CursorSink = std::function<void(CursorShape /*shape*/)>;

   /*
   *
   * Concrete Hover Cursor Controller Class
   *
   * Listens to pointer moves over the host, asks each injected rule in turn
   * what the element under the pointer deserves (first claim wins, Default if
   * none), and pushes the result to the injected sink. Holds only policy —
   * the rules, the host it listens on, and the sink it writes to — so it owns
   * no cursor and knows no concrete control. RAII revokers drop the
   * subscriptions when it dies. One instance per window owns it.
   *
   */
   class OHoverCursorController final : public IHoverCursorController
   {
      public:

         OHoverCursorController(winrt::Microsoft::UI::Xaml::UIElement host,
                                std::vector<std::unique_ptr<ICursorRule>> rules,
                                CursorSink sink);

         OHoverCursorController(const OHoverCursorController& controller) = delete;
         OHoverCursorController(OHoverCursorController&& controller) noexcept = delete;

         ~OHoverCursorController() noexcept override;

         OHoverCursorController& operator=(const OHoverCursorController& controller) = delete;
         OHoverCursorController& operator=(OHoverCursorController&& controller) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IHoverCursorController overrides
         //

         void refresh(void) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_CURSOR_OHOVERCURSORCONTROLLER_HPP
