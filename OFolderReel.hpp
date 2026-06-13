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

#ifndef TALAY_GUI_REEL_OFOLDERREEL_HPP
#define TALAY_GUI_REEL_OFOLDERREEL_HPP

#include "IFolderReel.hpp"

#include <memory>
#include <vector>

namespace talay::GUI::Reel
{
   /*
   *
   * Concrete Folder Reel Class
   *
   * Scans the open image's folder once into a cached path list and renders it
   * one page at a time through an anka Pager: each visible tile is a checkable
   * button carrying its path (Tag) and a decoded thumbnail above its name,
   * appended to the bound strip panel; tiles borrow their style and brushes from
   * the bound resource host so they match the XAML palette exactly. Opening an
   * image jumps to its page, the pager buttons turn pages without rescanning,
   * and only page moves rebuild tiles — so thumbnail decoding stays bounded by
   * the page size however large the folder is. The supported-format set is
   * injected once at construction (Dependency Inversion), the folder scanner and
   * thumbnailer it uses are shared singletons, and all WinUI state plus the
   * thumbnail-decoding plumbing are hidden behind a pimpl. One instance lives per
   * reel (not a singleton — a window owns it).
   *
   */
   class OFolderReel final : public IFolderReel
   {
      public:

         explicit OFolderReel(std::vector<std::wstring> supportedFormats);

         OFolderReel(const OFolderReel& reel) = delete;
         OFolderReel(OFolderReel&& reel) noexcept = delete;

         ~OFolderReel() noexcept override;

         OFolderReel& operator=(const OFolderReel& reel) = delete;
         OFolderReel& operator=(OFolderReel&& reel) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IFolderReel overrides
         //

         void attach(const ReelControls& controls) override;

         void show(const std::wstring& imagePath) override;

         void onImageActivated(ImageActivatedCallback callback) override;
         void onExpandRequested(ExpandRequestedCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_GUI_REEL_OFOLDERREEL_HPP
