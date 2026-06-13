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

#ifndef TALAY_GUI_REEL_REELCONTROLS_HPP
#define TALAY_GUI_REEL_REELCONTROLS_HPP

#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>

namespace talay::GUI::Reel
{
   // The XAML controls the folder reel works through, bound once at attach.
   // A parameter object (same shape as LibraryPanelControls), so growing the
   // reel's UI never widens the IFolderReel::attach signature.
   class ReelControls
   {
      public:

         // Vertical panel the thumbnail tiles are built into, and its scroll
         // host — manual page turns snap the host back to the page top.
         winrt::Microsoft::UI::Xaml::Controls::Panel        strip    {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::ScrollViewer scroller {nullptr};

         // Pager bar — pages bound how many thumbnails decode per rebuild.
         winrt::Microsoft::UI::Xaml::Controls::Button    pagePrev  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::Button    pageNext  {nullptr};
         winrt::Microsoft::UI::Xaml::Controls::TextBlock pageLabel {nullptr};

         // Element whose themed resources (tile style, brushes) the
         // code-built tiles borrow.
         winrt::Microsoft::UI::Xaml::FrameworkElement resourceHost {nullptr};
   };
}

#endif // TALAY_GUI_REEL_REELCONTROLS_HPP
