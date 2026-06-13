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

#ifndef ANKA_GUI_DIALOG_IDIALOGSERVICE_HPP
#define ANKA_GUI_DIALOG_IDIALOGSERVICE_HPP

#include <winrt/Microsoft.UI.Xaml.h>

#include "framework/Core/Object/IObject.hpp"
#include "DialogRequest.hpp"

namespace anka::GUI::Dialog
{
   /*
   * Abstract Dialog Service Interface
   *
   * Shows application-styled modal dialogs (message / question) in place of the
   * platform's stock boxes. The outcome of a button press is dependency-injected
   * as a std::function, so a caller never sees a dialog result type and stays
   * decoupled from the concrete WinUI implementation behind this interface.
   *
   */
   class IDialogService : public anka::Core::Object::IObject
   {
      public:

         virtual ~IDialogService() noexcept override = default;

         // The XAML island a dialog anchors onto. WinUI 3 desktop has no implicit
         // root, so this must be set before a dialog can be shown.
         virtual void setHost(const winrt::Microsoft::UI::Xaml::XamlRoot& host) = 0;

         // Inject the host's dialog chrome (style key + button captions) once, so
         // the framework dialog shows the app's look and language. Optional: an
         // unconfigured service yields stock-styled "OK / Yes / No" dialogs.
         virtual void configure(const DialogChrome& chrome) = 0;

         // One-button acknowledgement. onAcknowledged runs on press.
         virtual void showMessage(const DialogRequest& request,
                                  const DialogCallback& onAcknowledged = {}) = 0;

         // Two-button question. onAccepted runs on Yes, onRejected on No or any
         // dismissal (Esc, away-click).
         virtual void askQuestion(const DialogRequest& request,
                                  const DialogCallback& onAccepted,
                                  const DialogCallback& onRejected = {}) = 0;

         // Three-button choice between two named outcomes. onPrimary and
         // onSecondary run on the matching caption's press; onDismissed runs
         // on cancel or any dismissal (Esc, away-click), so backing out is
         // never mistaken for a choice.
         virtual void askChoice(const DialogRequest& request,
                                const DialogChoice& choice,
                                const DialogCallback& onPrimary,
                                const DialogCallback& onSecondary,
                                const DialogCallback& onDismissed = {}) = 0;
   };
}

#endif // ANKA_GUI_DIALOG_IDIALOGSERVICE_HPP
