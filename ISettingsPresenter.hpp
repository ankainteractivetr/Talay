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

#ifndef TALAY_SETTINGS_ISETTINGSPRESENTER_HPP
#define TALAY_SETTINGS_ISETTINGSPRESENTER_HPP

#include "SettingsState.hpp"
#include "framework/Core/Object/IObject.hpp"

namespace talay::Settings
{
   /*
   * Abstract Settings Presenter Interface (MVP)
   *
   * The logic half of the settings dialog: it answers what the dialog should
   * show (currentState / defaultState) and carries a confirmed snapshot into
   * the world (apply: ini persistence + shell registration). The view stays
   * a dumb control surface and never touches storage or the registry.
   *
   */
   class ISettingsPresenter : public anka::Core::Object::IObject
   {
      public:

         virtual ~ISettingsPresenter() noexcept override = default;

         // What the dialog shows when it opens / when "Defaults" is pressed.
         virtual SettingsState currentState(void) const = 0;
         virtual SettingsState defaultState(void) const = 0;

         // Persist a confirmed snapshot and report what changed.
         virtual SettingsApplyResult apply(const SettingsState& state) = 0;

         // Reset every setting to factory defaults — not just this dialog's
         // toggles but the window bounds, language and conversion directory —
         // reconcile the shell registration, and report what changed so the
         // window can react exactly as it does after an apply (restart prompt,
         // always-on-top). The confirmation prompt is the caller's to show.
         virtual SettingsApplyResult resetAllToDefaults(void) = 0;
   };
}

#endif // TALAY_SETTINGS_ISETTINGSPRESENTER_HPP
