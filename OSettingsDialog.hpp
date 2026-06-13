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

#ifndef TALAY_GUI_SETTINGS_OSETTINGSDIALOG_HPP
#define TALAY_GUI_SETTINGS_OSETTINGSDIALOG_HPP

#include "ISettingsPresenter.hpp"

#include <winrt/Microsoft.UI.Xaml.h>

#include <functional>
#include <memory>

namespace talay::GUI::Settings
{
   /*
   *
   * Concrete OSettingsDialog Class (MVP view)
   *
   * The settings modal: a code-composed, app-styled ContentDialog overlay
   * (same surface the dialog service uses), anchored on the host window's
   * XamlRoot. Strictly a control surface — every decision is delegated to
   * the injected presenter, and the two outcomes the host must act on
   * arrive as injected callbacks (Command), so the view never knows what
   * "apply" or "reset the library" actually mean.
   *
   */
   class OSettingsDialog final
   {
      public:

         struct Callbacks
         {
            // Runs after Save was pressed and the presenter applied the snapshot.
            std::function<void(const talay::Settings::SettingsApplyResult&)> onApplied;

            // Runs after the "Reset library" button closed the dialog; the host
            // owns the warning question and the actual reset.
            std::function<void(void)> onResetLibraryRequested;
         };

         explicit OSettingsDialog(talay::Settings::ISettingsPresenter& presenter);
         OSettingsDialog(const OSettingsDialog& object) = delete;
         OSettingsDialog(OSettingsDialog&& object) noexcept = delete;

         ~OSettingsDialog() noexcept;

         OSettingsDialog& operator=(const OSettingsDialog& object) = delete;
         OSettingsDialog& operator=(OSettingsDialog&& object) noexcept = delete;

         // Build and show the modal over 'host'. Re-entrant calls while it is
         // already on screen are ignored (one settings dialog at a time).
         void show(const winrt::Microsoft::UI::Xaml::XamlRoot& host, const Callbacks& callbacks);


      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_GUI_SETTINGS_OSETTINGSDIALOG_HPP
