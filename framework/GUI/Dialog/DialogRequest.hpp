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

#ifndef ANKA_GUI_DIALOG_DIALOGREQUEST_HPP
#define ANKA_GUI_DIALOG_DIALOGREQUEST_HPP

#include <functional>
#include <string>

namespace anka::GUI::Dialog
{
   // Visual intent of a dialog. It alone selects the header icon and accent
   // colour, so one themed template can express every message kind without the
   // caller ever touching a glyph or a brush.
   enum class DialogSeverity
   {
      Information,
      Success,
      Warning,
      Error,
      Question
   };

   // Everything a caller wants shown, and nothing about how it is shown. Kept an
   // aggregate on purpose: call sites read as C++20 designated initialisers,
   // e.g. {.title = L"...", .message = L"...", .severity = ...}.
   struct DialogRequest
   {
      std::wstring   title;
      std::wstring   message;
      DialogSeverity severity {DialogSeverity::Information};
   };

   // Injected behaviour for a button press (Command). An empty function is a
   // valid, intentional "do nothing".
   using DialogCallback = std::function<void()>;

   // The captions of a three-way choice: two named outcomes plus a cancel that
   // backs out without choosing. Same aggregate spirit as DialogRequest, so a
   // call site reads as designated initialisers; the defaults keep an
   // unlocalised call usable.
   struct DialogChoice
   {
      std::wstring primaryCaption   {L"Yes"};
      std::wstring secondaryCaption {L"No"};
      std::wstring cancelCaption    {L"Cancel"};
   };

   // Host-supplied chrome the service applies to every dialog, so the framework
   // carries no application specifics. 'styleKey' names an app ContentDialog Style
   // in Application.Resources (empty → the platform's stock chrome); the captions
   // are the button labels in the app's own language. The defaults make an
   // unconfigured service usable stand-alone.
   struct DialogChrome
   {
      std::wstring styleKey;
      std::wstring okCaption  {L"OK"};
      std::wstring yesCaption {L"Yes"};
      std::wstring noCaption  {L"No"};
   };
}

#endif // ANKA_GUI_DIALOG_DIALOGREQUEST_HPP
