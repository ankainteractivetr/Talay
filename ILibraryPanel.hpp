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

#ifndef TALAY_GUI_LIBRARY_ILIBRARYPANEL_HPP
#define TALAY_GUI_LIBRARY_ILIBRARYPANEL_HPP

#include "LibraryPanelControls.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <functional>
#include <string>

namespace talay::GUI::Library
{
   // Fired when a result row / tile is clicked, carrying the image's full path
   // so the host can open it. The panel never opens files itself.
   using OpenRequestedCallback = std::function<void(const std::wstring& /*path*/)>;

   // Fired when a row / tile's context menu asks to drop the image from the
   // library. The panel never mutates the library itself — the host performs
   // the removal and keeps the rest of its UI in sync.
   using RemoveRequestedCallback = std::function<void(const std::wstring& /*path*/)>;

   // Fired when a row / tile's context menu asks to convert the image. The
   // host owns the conversion dialog; the panel only reports the request.
   using ConvertRequestedCallback = std::function<void(const std::wstring& /*path*/)>;

   // Fired when a row / tile's context menu asks to drop the entry's
   // favorite mark (offered only on favorites). As with removal, the host
   // mutates the library and resyncs whatever else shows that state.
   using UnfavoriteRequestedCallback = std::function<void(const std::wstring& /*path*/)>;

   /*
   * Abstract Library Panel Interface
   *
   * Drives the image-library side panel: reads the filter controls into a
   * LibraryFilter as the user types and toggles (debounced live filtering),
   * queries the library, and renders the results page by page as a table or
   * a thumbnail grid. The host binds the controls once via attach(),
   * subscribes to the entry callbacks (open / convert / unfavorite /
   * remove), and forwards its Reset button to resetFilters(); all event
   * wiring, debounce timing, paging and result rendering stay hidden behind
   * the implementation.
   *
   */
   class ILibraryPanel : public anka::Core::Object::IObject
   {
      public:

         virtual ~ILibraryPanel() noexcept override = default;

         virtual void attach(const LibraryPanelControls& controls) = 0;

         // Rebuild the dynamic dropdowns (formats / folders found in the
         // library) and re-run the current filter.
         virtual void refresh(void) = 0;

         // Put every filter control back to its match-all state and re-query.
         virtual void resetFilters(void) = 0;

         virtual void onOpenRequested(OpenRequestedCallback callback) = 0;

         virtual void onRemoveRequested(RemoveRequestedCallback callback) = 0;

         virtual void onConvertRequested(ConvertRequestedCallback callback) = 0;

         virtual void onUnfavoriteRequested(UnfavoriteRequestedCallback callback) = 0;
   };
}

#endif // TALAY_GUI_LIBRARY_ILIBRARYPANEL_HPP
