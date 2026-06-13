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

#ifndef ANKA_GUI_DRAGDROP_OFILEDROPTARGET_HPP
#define ANKA_GUI_DRAGDROP_OFILEDROPTARGET_HPP

#include "IFileDropTarget.hpp"

#include <memory>

namespace anka::GUI::DragDrop
{
   /*
   *
   * Concrete File Drop Target Class
   *
   * Wires DragOver / Drop on the bound element: a drag carrying storage items
   * is offered a Copy (drag-over must answer synchronously, while the payload's
   * paths are only readable asynchronously — so per-file filtering waits for
   * the drop itself), and a drop hands the first file whose extension passes
   * the filter to the callback. The payload query is a coroutine that owns
   * copies of everything it touches, so a target that dies mid-flight leaves
   * no dangling state behind. One instance lives per drop surface (not a
   * singleton — a window owns it). All WinUI state and the event subscriptions
   * are hidden behind a pimpl, whose RAII revokers detach the handlers when
   * the target dies.
   *
   */
   class OFileDropTarget final : public IFileDropTarget
   {
      public:

         OFileDropTarget();

         OFileDropTarget(const OFileDropTarget& target) = delete;
         OFileDropTarget(OFileDropTarget&& target) noexcept = delete;

         ~OFileDropTarget() noexcept override;

         OFileDropTarget& operator=(const OFileDropTarget& target) = delete;
         OFileDropTarget& operator=(OFileDropTarget&& target) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IFileDropTarget overrides
         //

         void attach(const winrt::Microsoft::UI::Xaml::UIElement& dropSurface) override;

         void allowExtensions(const std::vector<std::wstring>& extensions) override;

         void onFileDropped(FileDroppedCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GUI_DRAGDROP_OFILEDROPTARGET_HPP
