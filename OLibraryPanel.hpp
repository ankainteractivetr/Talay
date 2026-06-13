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

#ifndef TALAY_GUI_LIBRARY_OLIBRARYPANEL_HPP
#define TALAY_GUI_LIBRARY_OLIBRARYPANEL_HPP

#include "ILibraryPanel.hpp"
#include "IImageLibrary.hpp"

#include <memory>

namespace talay::GUI::Library
{
   /*
   *
   * Concrete Library Panel Class
   *
   * The Mediator between the panel's XAML controls and the image library:
   * every control change funnels into one rebuild-filter-and-requery path
   * (text input debounced through a dispatcher timer so filtering feels live
   * without querying per keystroke), and the format / folder dropdowns are
   * rebuilt from what the library actually contains whenever it changes.
   * Results render one page at a time through an anka Pager (filter edits
   * jump to the first page, library changes keep the page, turning pages
   * never requeries). Rows and tiles are built in code and borrow the bound
   * resource host's styles, each carrying a right-click menu (open /
   * convert / unfavorite / remove) whose picks report through the host
   * callbacks, the thumbnail grid memoizes decoded WIC thumbnails per path,
   * and the library arrives by reference (Dependency Inversion) — the panel
   * neither creates nor owns it. One instance per window (not a singleton).
   *
   */
   class OLibraryPanel final : public ILibraryPanel
   {
      public:

         explicit OLibraryPanel(talay::Library::IImageLibrary& library);

         OLibraryPanel(const OLibraryPanel& panel) = delete;
         OLibraryPanel(OLibraryPanel&& panel) noexcept = delete;

         ~OLibraryPanel() noexcept override;

         OLibraryPanel& operator=(const OLibraryPanel& panel) = delete;
         OLibraryPanel& operator=(OLibraryPanel&& panel) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // ILibraryPanel overrides
         //

         void attach(const LibraryPanelControls& controls) override;

         void refresh(void) override;

         void resetFilters(void) override;

         void onOpenRequested(OpenRequestedCallback callback) override;

         void onRemoveRequested(RemoveRequestedCallback callback) override;

         void onConvertRequested(ConvertRequestedCallback callback) override;

         void onUnfavoriteRequested(UnfavoriteRequestedCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_GUI_LIBRARY_OLIBRARYPANEL_HPP
