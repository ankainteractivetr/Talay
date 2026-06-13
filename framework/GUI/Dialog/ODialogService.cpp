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
#include "ODialogService.hpp"

#include <winrt/Windows.UI.h>

using namespace anka::Core::Object;
using namespace anka::GUI::Dialog;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;

// -----------------------------------------------------------------------------
// View construction helpers — the dialog body is composed from these so each
// piece (icon, header, message) stays a one-responsibility builder. Surface and
// button chrome come from XAML; only the content varies per request.
// -----------------------------------------------------------------------------
namespace
{
   constexpr wchar_t k_iconFont[] {L"Segoe MDL2 Assets"};
   constexpr wchar_t k_titleFont[] {L"Georgia"};
   constexpr wchar_t k_bodyFont[] {L"Segoe UI"};

   Windows::UI::Color rgb(std::uint8_t r, std::uint8_t g, std::uint8_t b)
   {
      return Windows::UI::Color {255, r, g, b};
   }

   // Glyph that signals the dialog's intent (Segoe MDL2 Assets, as used app-wide):
   // Completed, Important, ErrorBadge, Unknown, Info.
   hstring severityGlyph(DialogSeverity severity)
   {
      switch (severity) {
         case DialogSeverity::Success:  return L"";
         case DialogSeverity::Warning:  return L"";
         case DialogSeverity::Error:    return L"";
         case DialogSeverity::Question: return L"";
         default:                       return L"";
      }
   }

   // Accent that tints the icon: cyan to reassure, gold to prompt, coral to warn.
   Windows::UI::Color severityColor(DialogSeverity severity)
   {
      switch (severity) {
         case DialogSeverity::Warning:  return rgb(0xE9, 0xB7, 0x65);
         case DialogSeverity::Error:    return rgb(0xFF, 0x6F, 0x6B);
         case DialogSeverity::Question: return rgb(0xFF, 0xD5, 0x89);
         default:                       return rgb(0x5A, 0xF2, 0xDD);
      }
   }

   FontIcon makeIcon(DialogSeverity severity)
   {
      FontIcon icon;
      icon.Glyph(severityGlyph(severity));
      icon.FontFamily(FontFamily {k_iconFont});
      icon.FontSize(20);
      icon.Foreground(SolidColorBrush {severityColor(severity)});
      return icon;
   }

   TextBlock makeHeaderText(const std::wstring& title)
   {
      TextBlock text;
      text.Text(hstring {title});
      text.FontFamily(FontFamily {k_titleFont});
      text.FontSize(15);
      text.CharacterSpacing(160);
      text.VerticalAlignment(VerticalAlignment::Center);
      text.Foreground(SolidColorBrush {rgb(0x5A, 0xF2, 0xDD)});
      return text;
   }

   StackPanel makeHeader(const DialogRequest& request)
   {
      StackPanel header;
      header.Orientation(Orientation::Horizontal);
      header.Spacing(11);
      header.Children().Append(makeIcon(request.severity));
      header.Children().Append(makeHeaderText(request.title));
      return header;
   }

   TextBlock makeBody(const std::wstring& message)
   {
      TextBlock body;
      body.Text(hstring {message});
      body.FontFamily(FontFamily {k_bodyFont});
      body.FontSize(13);
      body.TextWrapping(TextWrapping::WrapWholeWords);
      body.Foreground(SolidColorBrush {rgb(0xDB, 0xF7, 0xF1)});
      body.MaxWidth(420);
      return body;
   }
}

// -----------------------------------------------------------------------------
// ODialogService::Impl Definition
// -----------------------------------------------------------------------------
struct ODialogService::Impl
{
   ObjectIdentity m_identity {L"DialogServiceObject"};
   XamlRoot       m_host {nullptr};
   ContentDialog  m_active {nullptr};   // at most one dialog is live at a time
   DialogChrome   m_chrome {};          // host-injected style key + button captions

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

   void setHost(const XamlRoot& host)
   {
      m_host = host;
   }

   void configure(const DialogChrome& chrome)
   {
      m_chrome = chrome;
   }

   // The host-supplied ContentDialog style, looked up by key from the app's
   // resources; no key (or a missing one) silently falls back to stock chrome
   // rather than throwing.
   void applyStyle(const ContentDialog& dialog) const
   {
      if (m_chrome.styleKey.empty())
         return;

      auto resources {Application::Current().Resources()};
      auto key {box_value(hstring {m_chrome.styleKey})};
      if (resources.HasKey(key))
         dialog.Style(resources.Lookup(key).as<Style>());
   }

   ContentDialog makeDialog(const DialogRequest& request) const
   {
      ContentDialog dialog;
      dialog.XamlRoot(m_host);
      dialog.RequestedTheme(ElementTheme::Dark);
      dialog.Title(makeHeader(request));
      dialog.Content(makeBody(request.message));
      dialog.DefaultButton(ContentDialogButton::Primary);
      applyStyle(dialog);
      return dialog;
   }

   static void configureButtons(const ContentDialog& dialog,
                                const hstring& acceptText, const hstring& rejectText)
   {
      dialog.PrimaryButtonText(acceptText);
      if (!rejectText.empty())
         dialog.CloseButtonText(rejectText);
   }

   // Primary press is the accept path; everything else (No, Esc, away-click) is
   // the reject path. Either callback may be empty.
   static void route(ContentDialogResult result,
                     const DialogCallback& onAccept, const DialogCallback& onReject)
   {
      if (result == ContentDialogResult::Primary) {
         if (onAccept) onAccept();
         return;
      }
      if (onReject) onReject();
   }

   // Replace any open dialog with 'dialog' and hand back how the user closed
   // it. The singleton Impl keeps `this` alive across the suspension.
   Windows::Foundation::IAsyncOperation<ContentDialogResult> showExclusive(ContentDialog dialog)
   {
      if (m_active) m_active.Hide();
      m_active = dialog;

      const ContentDialogResult result {co_await dialog.ShowAsync()};

      if (m_active == dialog) m_active = nullptr;
      co_return result;
   }

   // The whole show: build, replace any open dialog, await the user, route the
   // outcome. By-value parameters keep the request and callbacks alive across
   // the suspension.
   fire_and_forget present(DialogRequest request, hstring acceptText, hstring rejectText,
                           DialogCallback onAccept, DialogCallback onReject)
   {
      ContentDialog dialog {makeDialog(request)};
      configureButtons(dialog, acceptText, rejectText);

      const ContentDialogResult result {co_await showExclusive(dialog)};
      route(result, onAccept, onReject);
   }

   static void configureChoiceButtons(const ContentDialog& dialog, const DialogChoice& choice)
   {
      dialog.PrimaryButtonText(hstring {choice.primaryCaption});
      dialog.SecondaryButtonText(hstring {choice.secondaryCaption});
      dialog.CloseButtonText(hstring {choice.cancelCaption});
   }

   // Each named button routes to its own outcome; everything else (cancel,
   // Esc, away-click) is a dismissal, never mistaken for a choice.
   static void routeChoice(ContentDialogResult result, const DialogCallback& onPrimary,
                           const DialogCallback& onSecondary, const DialogCallback& onDismissed)
   {
      if (result == ContentDialogResult::Primary) {
         if (onPrimary) onPrimary();
         return;
      }

      if (result == ContentDialogResult::Secondary) {
         if (onSecondary) onSecondary();
         return;
      }

      if (onDismissed) onDismissed();
   }

   // The three-button counterpart of present(), same lifetime reasoning.
   fire_and_forget presentChoice(DialogRequest request, DialogChoice choice,
                                 DialogCallback onPrimary, DialogCallback onSecondary,
                                 DialogCallback onDismissed)
   {
      ContentDialog dialog {makeDialog(request)};
      configureChoiceButtons(dialog, choice);

      const ContentDialogResult result {co_await showExclusive(dialog)};
      routeChoice(result, onPrimary, onSecondary, onDismissed);
   }

   void showMessage(const DialogRequest& request, const DialogCallback& onOk)
   {
      present(request, hstring {m_chrome.okCaption}, L"", onOk, nullptr);
   }

   void askQuestion(const DialogRequest& request,
                    const DialogCallback& onYes, const DialogCallback& onNo)
   {
      present(request, hstring {m_chrome.yesCaption}, hstring {m_chrome.noCaption}, onYes, onNo);
   }

   void askChoice(const DialogRequest& request, const DialogChoice& choice,
                  const DialogCallback& onPrimary, const DialogCallback& onSecondary,
                  const DialogCallback& onDismissed)
   {
      presentChoice(request, choice, onPrimary, onSecondary, onDismissed);
   }
};

// -----------------------------------------------------------------------------
// ODialogService Implementation
// -----------------------------------------------------------------------------
ODialogService::ODialogService() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
ODialogService::~ODialogService() noexcept = default;
// -----------------------------------------------------------------------------
ODialogService& ODialogService::getInstance()
{
   static ODialogService s_instance;
   return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& ODialogService::getName(void) const
{
   return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void ODialogService::setName(const std::wstring& name)
{
   m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t ODialogService::getId(void) const
{
   return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
void ODialogService::setHost(const winrt::Microsoft::UI::Xaml::XamlRoot& host)
{
   m_pImpl->setHost(host);
}
// -----------------------------------------------------------------------------
void ODialogService::configure(const DialogChrome& chrome)
{
   m_pImpl->configure(chrome);
}
// -----------------------------------------------------------------------------
void ODialogService::showMessage(const DialogRequest& request, const DialogCallback& onAcknowledged)
{
   m_pImpl->showMessage(request, onAcknowledged);
}
// -----------------------------------------------------------------------------
void ODialogService::askQuestion(const DialogRequest& request,
                                 const DialogCallback& onAccepted, const DialogCallback& onRejected)
{
   m_pImpl->askQuestion(request, onAccepted, onRejected);
}
// -----------------------------------------------------------------------------
void ODialogService::askChoice(const DialogRequest& request, const DialogChoice& choice,
                               const DialogCallback& onPrimary, const DialogCallback& onSecondary,
                               const DialogCallback& onDismissed)
{
   m_pImpl->askChoice(request, choice, onPrimary, onSecondary, onDismissed);
}
// -----------------------------------------------------------------------------
