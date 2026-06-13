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

#ifndef ANKA_GUI_MENU_MENUCOMMAND_HPP
#define ANKA_GUI_MENU_MENUCOMMAND_HPP

#include <functional>
#include <string>

namespace anka::GUI::Menu
{
   // Injected behaviour for a menu pick (Command). An empty function is a
   // valid, intentional "do nothing".
   using MenuAction = std::function<void()>;

   // What one descriptor stands for: a clickable command, or a visual
   // divider between command groups. Dividers ignore label / glyph / action.
   enum class MenuItemKind
   {
      Command,
      Separator
   };

   // Everything a caller wants on one menu row, and nothing about how it is
   // rendered. Kept an aggregate on purpose: call sites read as C++20
   // designated initialisers, e.g. {.label = L"Open", .action = ...}.
   // 'glyph' names a Segoe MDL2 Assets icon; left empty, the row is text-only.
   struct MenuCommand
   {
      std::wstring label;
      std::wstring glyph;
      MenuAction   action;
      MenuItemKind kind {MenuItemKind::Command};
   };

   // Reads better at call sites than an all-empty aggregate.
   inline MenuCommand menuSeparator(void)
   {
      return {.kind = MenuItemKind::Separator};
   }
}

#endif // ANKA_GUI_MENU_MENUCOMMAND_HPP
