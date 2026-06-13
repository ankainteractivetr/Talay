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

#ifndef TALAY_GUI_LOCALIZEDDIALOG_HPP
#define TALAY_GUI_LOCALIZEDDIALOG_HPP

#include "framework/GUI/Dialog/IDialogService.hpp"
#include "framework/GUI/Dialog/DialogRequest.hpp"

#include <string_view>

namespace talay::GUI
{
   // The app's most common notice: a themed message box whose title and body
   // are both resource ids, resolved in the current language. It folds the two
   // steps every such notice used to repeat — reaching for the localization
   // service, then handing showMessage the looked-up strings — into one line.
   //
   // The dialog service is passed in rather than fetched here: a window hands
   // over its own dialogs() so the message anchors on that window's live
   // XamlRoot. This keeps the helper a thin shaper over the service, not a
   // second owner of the dialog stack. Notices whose body is built at runtime
   // (vformat with counts) still call showMessage directly — only the all-keys
   // case belongs here.
   void showLocalizedMessage(anka::GUI::Dialog::IDialogService& service,
                             std::wstring_view titleKey, std::wstring_view bodyKey,
                             anka::GUI::Dialog::DialogSeverity severity);
}

#endif // TALAY_GUI_LOCALIZEDDIALOG_HPP
