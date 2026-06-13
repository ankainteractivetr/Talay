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
#include "OClickableCursorRule.hpp"

#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>

using namespace anka::GUI::Cursor;

using winrt::Microsoft::UI::Xaml::Controls::Control;
using winrt::Microsoft::UI::Xaml::Controls::Primitives::ButtonBase;
using winrt::Microsoft::UI::Xaml::Controls::Primitives::SelectorItem;
using winrt::Microsoft::UI::Xaml::DependencyObject;
using winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper;

namespace
{
   // A node counts as clickable when it is an enabled button or grid/list tile;
   // a disabled one offers nothing to click, so it is not claimed.
   bool isClickable(const DependencyObject& node)
   {
      if (!node.try_as<ButtonBase>() && !node.try_as<SelectorItem>())
         return false;

      const auto control {node.try_as<Control>()};
      return !control || control.IsEnabled();
   }
}

// -----------------------------------------------------------------------------
// OClickableCursorRule Implementation
// -----------------------------------------------------------------------------
std::optional<CursorShape> OClickableCursorRule::resolve(const DependencyObject& hovered) const
{
   for (DependencyObject node {hovered}; node; node = VisualTreeHelper::GetParent(node))
      if (isClickable(node))
         return CursorShape::Hand;

   return std::nullopt;
}
// -----------------------------------------------------------------------------
