#include "pch.h"
#include "OCursorHost.h"
#if __has_include("OCursorHost.g.cpp")
#include "OCursorHost.g.cpp"
#endif

#include <winrt/Microsoft.UI.Input.h>

using namespace anka::GUI::Cursor;

using winrt::Microsoft::UI::Input::InputSystemCursor;
using winrt::Microsoft::UI::Input::InputSystemCursorShape;

namespace
{
   // The system cursor each abstract shape maps to; Default is the plain arrow.
   InputSystemCursorShape toSystemShape(CursorShape shape)
   {
      switch (shape) {
         case CursorShape::Hand: return InputSystemCursorShape::Hand;
         case CursorShape::Move: return InputSystemCursorShape::SizeAll;
         default:                return InputSystemCursorShape::Arrow;
      }
   }
}

namespace winrt::Talay::implementation
{
   void OCursorHost::showCursor(CursorShape shape)
   {
      ProtectedCursor(InputSystemCursor::Create(toSystemShape(shape)));
   }
}
