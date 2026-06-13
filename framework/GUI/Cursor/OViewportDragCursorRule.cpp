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
#include "OViewportDragCursorRule.hpp"

#include <winrt/Microsoft.UI.Xaml.Media.h>

#include <utility>

using namespace anka::GUI::Cursor;

using winrt::Microsoft::UI::Xaml::DependencyObject;
using winrt::Microsoft::UI::Xaml::UIElement;
using winrt::Microsoft::UI::Xaml::Media::VisualTreeHelper;

// -----------------------------------------------------------------------------
// OViewportDragCursorRule Implementation
// -----------------------------------------------------------------------------
OViewportDragCursorRule::OViewportDragCursorRule(UIElement surface, DragPredicate isDraggable) :
   m_surface     {std::move(surface)},
   m_isDraggable {std::move(isDraggable)}
{}
// -----------------------------------------------------------------------------
std::optional<CursorShape> OViewportDragCursorRule::resolve(const DependencyObject& hovered) const
{
   if (!m_isDraggable || !m_isDraggable())
      return std::nullopt;

   for (DependencyObject node {hovered}; node; node = VisualTreeHelper::GetParent(node))
      if (node == m_surface)
         return CursorShape::Move;

   return std::nullopt;
}
// -----------------------------------------------------------------------------
