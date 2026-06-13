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

#ifndef ANKA_GUI_DRAGDROP_IFILEDROPTARGET_HPP
#define ANKA_GUI_DRAGDROP_IFILEDROPTARGET_HPP

#include <winrt/Microsoft.UI.Xaml.h>

#include <functional>
#include <string>
#include <vector>

#include "framework/Core/Object/IObject.hpp"

namespace anka::GUI::DragDrop
{
   // Observer hook fired for each file the user drops onto the bound surface
   // that passes the extension filter; the host decides what opening it means.
   using FileDroppedCallback = std::function<void(const std::wstring& /*path*/)>;

   /*
   * Abstract File Drop Target Interface
   *
   * Turns a XAML element into a shell drop target: the host binds the surface
   * once via attach(), names the file extensions it accepts, and receives the
   * dropped file through the callback. The concrete WinUI machinery (drag-over
   * acceptance, the asynchronous payload query) is hidden behind the
   * implementation, so callers depend only on this abstraction.
   *
   */
   class IFileDropTarget : public anka::Core::Object::IObject
   {
      public:

         virtual ~IFileDropTarget() noexcept override = default;

         // Start accepting file drags over 'dropSurface' (AllowDrop is raised
         // here, so the XAML needs no change). Idempotent: a second call
         // rebinds to the new element.
         virtual void attach(const winrt::Microsoft::UI::Xaml::UIElement& dropSurface) = 0;

         // The dotted extensions a drop may deliver, any case. An empty list
         // — the default — lets every file through.
         virtual void allowExtensions(const std::vector<std::wstring>& extensions) = 0;

         virtual void onFileDropped(FileDroppedCallback callback) = 0;
   };
}

#endif // ANKA_GUI_DRAGDROP_IFILEDROPTARGET_HPP
