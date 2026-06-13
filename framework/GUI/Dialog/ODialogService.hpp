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

#ifndef ANKA_GUI_DIALOG_ODIALOGSERVICE_HPP
#define ANKA_GUI_DIALOG_ODIALOGSERVICE_HPP

#include "IDialogService.hpp"

#include <memory>

namespace anka::GUI::Dialog
{
   /*
   *
   * Concrete Dialog Service Class
   *
   * A facade over a themed WinUI ContentDialog. The look lives entirely in XAML
   * (Styles/Theme.xaml); this class owns only behaviour — building the dialog
   * and routing its outcome to the injected callbacks. Single instance per
   * process (one UI thread, one window), matching the other framework UI
   * services. All WinUI state is hidden behind a pimpl.
   *
   */
   class ODialogService final : public IDialogService
   {
      public:

         // Singleton
         static ODialogService& getInstance();

         ODialogService(const ODialogService& service) = delete;
         ODialogService(ODialogService&& service) noexcept = delete;

         ~ODialogService() noexcept override;

         ODialogService& operator=(const ODialogService& service) = delete;
         ODialogService& operator=(ODialogService&& service) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IDialogService overrides
         //

         void setHost(const winrt::Microsoft::UI::Xaml::XamlRoot& host) override;

         void configure(const DialogChrome& chrome) override;

         void showMessage(const DialogRequest& request,
                          const DialogCallback& onAcknowledged = {}) override;

         void askQuestion(const DialogRequest& request,
                          const DialogCallback& onAccepted,
                          const DialogCallback& onRejected = {}) override;

         void askChoice(const DialogRequest& request,
                        const DialogChoice& choice,
                        const DialogCallback& onPrimary,
                        const DialogCallback& onSecondary,
                        const DialogCallback& onDismissed = {}) override;

      private:

         // Singleton COTOR
         ODialogService();

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_DIALOG_ODIALOGSERVICE_HPP
