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

#ifndef TALAY_GUI_REEL_IFOLDERREEL_HPP
#define TALAY_GUI_REEL_IFOLDERREEL_HPP

#include "ReelControls.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <functional>
#include <string>

namespace talay::GUI::Reel
{
   // Fired when a thumbnail is clicked, carrying the image's full path so the host
   // can open it. The reel never opens files itself — it only reports the request.
   using ImageActivatedCallback = std::function<void(const std::wstring& /*path*/)>;

   // Fired when the strip was just rebuilt for a new folder, so the host can reveal
   // a collapsed panel. The reel does not own the panel's open/closed chrome.
   using ExpandRequestedCallback = std::function<void(void)>;

   /*
   * Abstract Folder Reel Interface
   *
   * Shows the images that share the open image's folder as a strip of clickable
   * thumbnails, keeping the open one highlighted and scrolled into view. The
   * strip is paged: only one page of tiles exists at a time (so a huge folder
   * never floods the UI thread with thumbnail decodes), and the bound pager bar
   * turns pages over the cached folder scan. The host binds the controls once
   * via attach(), subscribes to the two callbacks, then calls show() each time
   * an image opens; the reel rescans only when the folder changes and re-renders
   * only when the visible page moves (so tiles survive opening a sibling).
   * Thumbnail decoding and all the tile-building WinUI machinery stay hidden
   * behind the implementation, so the host depends only on this abstraction.
   *
   */
   class IFolderReel : public anka::Core::Object::IObject
   {
      public:

         virtual ~IFolderReel() noexcept override = default;

         // Bind the XAML controls the reel renders through (tile strip, pager
         // bar, themed-resource host) and wire the pager buttons. Call once.
         virtual void attach(const ReelControls& controls) = 0;

         // Reflect 'imagePath' in the reel: rescan if its folder changed (raising
         // onExpandRequested), page to its tile, then highlight and scroll to it.
         virtual void show(const std::wstring& imagePath) = 0;

         virtual void onImageActivated(ImageActivatedCallback callback) = 0;
         virtual void onExpandRequested(ExpandRequestedCallback callback) = 0;
   };
}

#endif // TALAY_GUI_REEL_IFOLDERREEL_HPP
