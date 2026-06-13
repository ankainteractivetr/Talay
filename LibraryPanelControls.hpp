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

#ifndef TALAY_GUI_LIBRARY_LIBRARYPANELCONTROLS_HPP
#define TALAY_GUI_LIBRARY_LIBRARYPANELCONTROLS_HPP

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace talay::GUI::Library
{
   // The XAML controls the library panel works through, bound once at attach.
   // A parameter object (the window's generated accessors hand these over in
   // one move), so the panel's interface never grows a 20-argument attach().
   class LibraryPanelControls
   {
      public:

         // Filter sidebar
         winrt::Microsoft::UI::Xaml::Controls::TextBox            nameBox        {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::DropDownButton     formatDropDown {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::TextBlock          formatSummary  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::StackPanel         formatList     {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::CalendarDatePicker dateFrom       {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::CalendarDatePicker dateTo         {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::NumberBox          sizeMin        {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::NumberBox          sizeMax        {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::NumberBox          widthMin       {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::NumberBox          widthMax       {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::NumberBox          heightMin      {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::NumberBox          heightMax      {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::CheckBox           stills         {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::CheckBox           animated       {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::CheckBox           favoritesOnly  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::ComboBox           folderBox      {nullptr};

         // Results surface
         winrt::Microsoft::UI::Xaml::Controls::ComboBox   displayMode {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::ComboBox   sortMode    {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::TextBlock  itemCount   {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::TextBlock  resultCount {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::TextBlock  emptyLabel  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::Grid       tableView   {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::StackPanel tableRows   {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::GridView   thumbGrid   {nullptr};

         // Pager bar
         winrt::Microsoft::UI::Xaml::Controls::ComboBox  pageSize  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::Button    pagePrev  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::Button    pageNext  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::TextBlock pageLabel {nullptr};

         // Element whose themed resources (brushes, row/tile styles) the
         // code-built rows and tiles borrow.
         winrt::Microsoft::UI::Xaml::FrameworkElement resourceHost {nullptr};
   };
}

#endif // TALAY_GUI_LIBRARY_LIBRARYPANELCONTROLS_HPP
