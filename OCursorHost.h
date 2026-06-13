#pragma once

#include "OCursorHost.g.h"
#include "framework/GUI/Cursor/CursorShape.hpp"

namespace winrt::Talay::implementation
{
    // A grid that exists only to own a cursor. UIElement.ProtectedCursor — the
    // one WinUI hook for a custom cursor — is reachable solely from a subclass,
    // so the window wraps its content in this and lets the hover controller
    // drive the shape. Set here, the cursor serves the whole subtree below
    // (any descendant that sets its own still wins).
    struct OCursorHost : OCursorHostT<OCursorHost>
    {
        OCursorHost() = default;

        // Not projected: a plain C++ enum crosses no ABI, so the controller's
        // sink calls this through the implementation type, not over WinRT.
        void showCursor(anka::GUI::Cursor::CursorShape shape);
    };
}

namespace winrt::Talay::factory_implementation
{
    struct OCursorHost : OCursorHostT<OCursorHost, implementation::OCursorHost>
    {
    };
}
