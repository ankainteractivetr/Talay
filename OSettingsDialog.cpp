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
#include "OSettingsDialog.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"

#include <winrt/Windows.UI.h>

using namespace talay::GUI::Settings;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;

using anka::Core::Localization::OLocalizationService;
using talay::Localization::Language;
using talay::Settings::ISettingsPresenter;
using talay::Settings::SettingsState;

// -----------------------------------------------------------------------------
// View vocabulary -- the palette and small builders the dialog is composed
// from, mirroring the dialog service so both surfaces read as one family.
// -----------------------------------------------------------------------------
namespace
{
   constexpr wchar_t k_titleFont[] {L"Georgia"};
   constexpr wchar_t k_bodyFont[]  {L"Segoe UI"};
   constexpr wchar_t k_styleKey[]  {L"TalayContentDialogStyle"};

   Windows::UI::Color rgba(std::uint8_t a, std::uint8_t r, std::uint8_t g, std::uint8_t b)
   {
      return Windows::UI::Color {a, r, g, b};
   }

   Windows::UI::Color foam(void)  { return rgba(255, 0xDB, 0xF7, 0xF1); }
   Windows::UI::Color dim(void)   { return rgba(255, 0x6F, 0x9A, 0xA3); }
   Windows::UI::Color cyan(void)  { return rgba(255, 0x5A, 0xF2, 0xDD); }
   Windows::UI::Color coral(void) { return rgba(255, 0xFF, 0x6F, 0x6B); }

   // The language combo's fixed row order: 0 = follow the OS, 1 = Turkish,
   // 2 = English. The two converters below are the only place that knows it.
   int32_t indexOf(std::optional<Language> language)
   {
      if (!language)
         return 0;

      return *language == Language::Turkish ? 1 : 2;
   }

   std::optional<Language> languageAt(int32_t index)
   {
      if (index == 1) return Language::Turkish;
      if (index == 2) return Language::English;
      return std::nullopt;
   }

   TextBlock makeTitle(const std::wstring& text)
   {
      TextBlock title;
      title.Text(hstring {text});
      title.FontFamily(FontFamily {k_titleFont});
      title.FontSize(15);
      title.CharacterSpacing(220);
      title.Foreground(SolidColorBrush {cyan()});
      return title;
   }

   TextBlock makeSectionHeader(const std::wstring& text)
   {
      TextBlock header;
      header.Text(hstring {text});
      header.FontFamily(FontFamily {k_bodyFont});
      header.FontSize(10);
      header.CharacterSpacing(180);
      header.Foreground(SolidColorBrush {dim()});
      return header;
   }

   TextBlock makeRowLabel(const std::wstring& text)
   {
      TextBlock label;
      label.Text(hstring {text});
      label.FontFamily(FontFamily {k_bodyFont});
      label.FontSize(13);
      label.Foreground(SolidColorBrush {foam()});
      label.VerticalAlignment(VerticalAlignment::Center);
      return label;
   }

   TextBlock makeHint(const std::wstring& text)
   {
      TextBlock hint;
      hint.Text(hstring {text});
      hint.FontFamily(FontFamily {k_bodyFont});
      hint.FontSize(11);
      hint.Foreground(SolidColorBrush {dim()});
      hint.TextWrapping(TextWrapping::WrapWholeWords);
      hint.MaxWidth(380);
      return hint;
   }

   ColumnDefinition starColumn(void)
   {
      ColumnDefinition column;
      column.Width(GridLengthHelper::FromValueAndType(1, GridUnitType::Star));
      return column;
   }

   ColumnDefinition autoColumn(void)
   {
      ColumnDefinition column;
      column.Width(GridLengthHelper::Auto());
      return column;
   }

   // One settings row: caption on the left, its control flush right.
   Grid makeRow(const std::wstring& caption, const FrameworkElement& control)
   {
      Grid row;
      row.ColumnDefinitions().Append(starColumn());
      row.ColumnDefinitions().Append(autoColumn());

      TextBlock label {makeRowLabel(caption)};
      Grid::SetColumn(label, 0);
      Grid::SetColumn(control, 1);
      row.Children().Append(label);
      row.Children().Append(control);
      return row;
   }

   void appendItem(const ComboBox& box, const std::wstring& caption)
   {
      ComboBoxItem item;
      item.Content(box_value(hstring {caption}));
      box.Items().Append(item);
   }

   ComboBox makeLanguageBox(void)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      ComboBox box;
      box.MinWidth(170);
      appendItem(box, loc.text(L"SettingsLangAuto"));
      appendItem(box, loc.text(L"SettingsLangTurkish"));
      appendItem(box, loc.text(L"SettingsLangEnglish"));
      return box;
   }

   ToggleSwitch makeToggle(void)
   {
      ToggleSwitch toggle;
      toggle.OnContent(box_value(hstring {L""}));
      toggle.OffContent(box_value(hstring {L""}));
      toggle.HorizontalAlignment(HorizontalAlignment::Right);
      toggle.MinWidth(0);
      return toggle;
   }

   // The destructive action wears the warning colour, nothing else does.
   Button makeResetButton(const std::wstring& caption)
   {
      Button button;
      button.Content(box_value(hstring {caption}));
      button.FontSize(12);
      button.Foreground(SolidColorBrush {coral()});
      button.BorderBrush(SolidColorBrush {rgba(0x99, 0xFF, 0x6F, 0x6B)});
      button.BorderThickness(Thickness {1, 1, 1, 1});
      button.CornerRadius(CornerRadius {7, 7, 7, 7});
      button.Padding(Thickness {14, 7, 14, 7});
      return button;
   }

   StackPanel makeSection(const std::wstring& header)
   {
      StackPanel section;
      section.Spacing(10);
      section.Children().Append(makeSectionHeader(header));
      return section;
   }

   // The app ContentDialog style, looked up the same way the dialog service
   // does it; a missing key silently falls back to stock chrome.
   void applyStyle(const ContentDialog& dialog)
   {
      auto resources {Application::Current().Resources()};
      auto key {box_value(hstring {k_styleKey})};
      if (resources.HasKey(key))
         dialog.Style(resources.Lookup(key).as<Style>());
   }
}

// -----------------------------------------------------------------------------
// OSettingsDialog::Impl Definition
// -----------------------------------------------------------------------------
struct OSettingsDialog::Impl
{
   ISettingsPresenter& m_presenter;

   // One dialog at a time: ShowAsync throws if a second overlay opens, so
   // re-entrant show() calls are ignored while this is true.
   bool m_active {false};

   explicit Impl(ISettingsPresenter& presenter) :
      m_presenter {presenter}
   {}

   // Every editable control in one place, so snapshots can be loaded into the
   // view and read back without the builders knowing about state at all.
   struct Controls
   {
      ComboBox     language     {nullptr};
      ToggleSwitch alwaysOnTop  {nullptr};
      ToggleSwitch fileTypes    {nullptr};
      ToggleSwitch autoAdd      {nullptr};
      Button       resetLibrary {nullptr};
   };

   static Controls makeControls(void)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      Controls controls;
      controls.language     = makeLanguageBox();
      controls.alwaysOnTop  = makeToggle();
      controls.fileTypes    = makeToggle();
      controls.autoAdd      = makeToggle();
      controls.resetLibrary = makeResetButton(loc.text(L"SettingsResetLibraryBtn"));
      return controls;
   }

   static void applyState(const Controls& controls, const SettingsState& state)
   {
      controls.language.SelectedIndex(indexOf(state.language));
      controls.alwaysOnTop.IsOn(state.alwaysOnTop);
      controls.fileTypes.IsOn(state.fileTypesRegistered);
      controls.autoAdd.IsOn(state.autoAddToLibrary);
   }

   static SettingsState stateFrom(const Controls& controls)
   {
      return {.language            = languageAt(controls.language.SelectedIndex()),
              .alwaysOnTop         = controls.alwaysOnTop.IsOn(),
              .autoAddToLibrary    = controls.autoAdd.IsOn(),
              .fileTypesRegistered = controls.fileTypes.IsOn()};
   }

   static StackPanel makeGeneralSection(const Controls& controls, const OLocalizationService& loc)
   {
      StackPanel section {makeSection(loc.text(L"SettingsSectionGeneral"))};
      section.Children().Append(makeRow(loc.text(L"SettingsLanguageLabel"), controls.language));
      section.Children().Append(makeRow(loc.text(L"SettingsAlwaysOnTop"), controls.alwaysOnTop));
      return section;
   }

   static StackPanel makeFileTypesSection(const Controls& controls, const OLocalizationService& loc)
   {
      StackPanel section {makeSection(loc.text(L"SettingsSectionFileTypes"))};
      section.Children().Append(makeRow(loc.text(L"SettingsRegisterTypes"), controls.fileTypes));
      section.Children().Append(makeHint(loc.text(L"SettingsRegisterTypesHint")));
      return section;
   }

   static StackPanel makeLibrarySection(const Controls& controls, const OLocalizationService& loc)
   {
      StackPanel section {makeSection(loc.text(L"SettingsSectionLibrary"))};
      section.Children().Append(makeRow(loc.text(L"SettingsAutoAdd"), controls.autoAdd));
      section.Children().Append(controls.resetLibrary);
      return section;
   }

   static StackPanel makeContent(const Controls& controls)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      StackPanel content;
      content.Spacing(18);
      content.MinWidth(380);
      content.Children().Append(makeGeneralSection(controls, loc));
      content.Children().Append(makeFileTypesSection(controls, loc));
      content.Children().Append(makeLibrarySection(controls, loc));
      return content;
   }

   static void configureButtons(const ContentDialog& dialog)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      dialog.PrimaryButtonText(hstring {loc.text(L"SettingsSave")});
      dialog.SecondaryButtonText(hstring {loc.text(L"SettingsDefaults")});
      dialog.CloseButtonText(hstring {loc.text(L"SettingsCancel")});
      dialog.DefaultButton(ContentDialogButton::Primary);
   }

   static ContentDialog makeDialog(const XamlRoot& host, const Controls& controls)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      ContentDialog dialog;
      dialog.XamlRoot(host);
      dialog.RequestedTheme(ElementTheme::Dark);
      dialog.Title(makeTitle(loc.text(L"SettingsTitle")));
      dialog.Content(makeContent(controls));
      configureButtons(dialog);
      applyStyle(dialog);
      return dialog;
   }

   // "Defaults" must not close the dialog: cancel the dismissal and reload
   // the controls with the factory snapshot instead.
   void wireDefaultsButton(const ContentDialog& dialog, const Controls& controls)
   {
      dialog.SecondaryButtonClick(
         [this, controls](ContentDialog const&, ContentDialogButtonClickEventArgs const& args) {
            args.Cancel(true);
            applyState(controls, m_presenter.defaultState());
         });
   }

   // The destructive flow leaves the dialog: hide it, then let the host run its
   // warning question. The question is a second ContentDialog, so it is deferred a
   // dispatcher tick — past this one's close — or WinUI's one-dialog-per-XamlRoot
   // rule makes it throw into the void. The weak dialog reference avoids a cycle.
   static void wireResetButton(const Controls& controls, const ContentDialog& dialog,
                               const Callbacks& callbacks)
   {
      controls.resetLibrary.Click(
         [weak = winrt::make_weak(dialog), onReset = callbacks.onResetLibraryRequested](auto const&, auto const&) {
            const auto live {weak.get()};
            if (!live || !onReset)
               return;

            live.Hide();
            live.DispatcherQueue().TryEnqueue([onReset] { onReset(); });
         });
   }

   ContentDialog buildView(const XamlRoot& host, const Controls& controls, const Callbacks& callbacks)
   {
      ContentDialog dialog {makeDialog(host, controls)};
      wireDefaultsButton(dialog, controls);
      wireResetButton(controls, dialog, callbacks);
      return dialog;
   }

   // By-value parameters keep host and callbacks alive across the suspension;
   // the Impl itself is kept alive by the window that owns the dialog object.
   fire_and_forget run(XamlRoot host, Callbacks callbacks)
   {
      m_active = true;
      Controls controls {makeControls()};
      applyState(controls, m_presenter.currentState());

      ContentDialog dialog {buildView(host, controls, callbacks)};
      const ContentDialogResult result {co_await dialog.ShowAsync()};
      m_active = false;

      if (result != ContentDialogResult::Primary || !callbacks.onApplied)
         co_return;

      // Let this dialog fully close before the host callback runs: WinUI allows
      // only one ContentDialog per XamlRoot, so an apply that opens another (the
      // restart prompt on a language change) would otherwise throw into the void.
      co_await wil::resume_foreground(dialog.DispatcherQueue());
      callbacks.onApplied(m_presenter.apply(stateFrom(controls)));
   }
};

// -----------------------------------------------------------------------------
// OSettingsDialog Implementation
// -----------------------------------------------------------------------------
OSettingsDialog::OSettingsDialog(ISettingsPresenter& presenter) :
   m_pImpl {std::make_unique<Impl>(presenter)}
{}
// -----------------------------------------------------------------------------
OSettingsDialog::~OSettingsDialog() noexcept = default;
// -----------------------------------------------------------------------------
void OSettingsDialog::show(const winrt::Microsoft::UI::Xaml::XamlRoot& host, const Callbacks& callbacks)
{
   if (m_pImpl->m_active)
      return;

   m_pImpl->run(host, callbacks);
}
// -----------------------------------------------------------------------------
