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

#ifndef TALAY_SETTINGS_SETTINGSSTATE_HPP
#define TALAY_SETTINGS_SETTINGSSTATE_HPP

#include "Language.hpp"

#include <optional>

namespace talay::Settings
{
   // Everything the settings dialog edits, gathered into one value object so
   // the view and the presenter exchange whole snapshots (Memento): Save
   // applies a snapshot, Cancel simply drops it, and "Defaults" is just this
   // struct's own initialisers loaded back into the controls.
   struct SettingsState
   {
      std::optional<talay::Localization::Language> language {};   // empty = follow the OS
      bool alwaysOnTop         {false};
      bool autoAddToLibrary    {true};
      bool fileTypesRegistered {true};
   };

   // What an apply actually changed — the window reacts to it: always-on-top
   // lands live, a language change earns a restart prompt.
   struct SettingsApplyResult
   {
      bool languageChanged {false};
      bool alwaysOnTop     {false};
   };
}

#endif // TALAY_SETTINGS_SETTINGSSTATE_HPP
