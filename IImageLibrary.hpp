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

#ifndef TALAY_LIBRARY_IIMAGELIBRARY_HPP
#define TALAY_LIBRARY_IIMAGELIBRARY_HPP

#include "LibraryEntry.hpp"
#include "LibraryFilter.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace talay::Library
{
   // Fired after any mutation (add / remove / favorite), so views re-query
   // instead of tracking individual changes.
   using LibraryChangedCallback = std::function<void(void)>;

   /*
   * Abstract Image Library Interface (Repository + Facade)
   *
   * The one seam the GUI talks to about the user's image library: mutations
   * write through to persistent storage, reads answer from memory, and
   * query() runs a LibraryFilter over the collection. How entries persist
   * (SQLite today) and how filters match (specifications today) stay
   * implementation details behind this interface.
   *
   */
   class IImageLibrary : public anka::Core::Object::IObject
   {
      public:

         virtual ~IImageLibrary() noexcept override = default;

         // Open (creating on first run) the backing database; false leaves an
         // empty, non-persisting library the app can still run against.
         virtual bool open(const std::wstring& databasePath) = 0;

         virtual bool add(const LibraryEntry& entry) = 0;
         virtual bool remove(const std::wstring& filePath) = 0;
         virtual bool setFavorite(const std::wstring& filePath, bool favorite) = 0;

         // Drop many entries in one batch — one store write, one change
         // notification — so a background reconciliation that prunes every
         // vanished file refreshes the views exactly once.
         virtual bool removePaths(const std::vector<std::wstring>& filePaths) = 0;

         // Drop every entry, favorites included (the settings dialog's "reset
         // library"). The image files on disk are untouched.
         virtual bool clear(void) = 0;

         virtual bool contains(const std::wstring& filePath) const = 0;
         virtual bool isFavorite(const std::wstring& filePath) const = 0;

         virtual std::size_t entryCount(void) const = 0;

         // A snapshot of every entry's backing-file path, so a background sweep
         // can verify them off the UI thread without holding the library.
         virtual std::vector<std::wstring> allPaths(void) const = 0;

         virtual std::vector<LibraryEntry> query(const LibraryFilter& filter) const = 0;

         // The distinct formats / source folders present in the library, each
         // sorted — what the panel's dynamic filter dropdowns are built from.
         virtual std::vector<std::wstring> availableFormats(void) const = 0;
         virtual std::vector<std::wstring> availableFolders(void) const = 0;

         virtual void onChanged(LibraryChangedCallback callback) = 0;
   };
}

#endif // TALAY_LIBRARY_IIMAGELIBRARY_HPP
