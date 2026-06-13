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

#ifndef TALAY_LIBRARY_STORE_ILIBRARYSTORE_HPP
#define TALAY_LIBRARY_STORE_ILIBRARYSTORE_HPP

#include "LibraryEntry.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace talay::Library::Store
{
   /*
   * Abstract Library Store Interface
   *
   * Pure persistence for library entries: open a database file once, load
   * everything, and write through single-row mutations. The store knows
   * nothing about filtering or the GUI — querying happens in memory above it
   * (Dependency Inversion: the repository owns this seam, so the SQL engine
   * behind it can be swapped without touching anything else).
   *
   * Every call is throw-free: a store that failed to open simply reports
   * failure / yields nothing, and the app keeps running without a library.
   *
   */
   class ILibraryStore : public anka::Core::Object::IObject
   {
      public:

         virtual ~ILibraryStore() noexcept override = default;

         // Open (creating on first run) the database at 'databasePath'.
         virtual bool open(const std::wstring& databasePath) = 0;

         virtual std::vector<LibraryEntry> loadAll(void) = 0;

         // Persist a new entry; returns its database id, 0 on failure.
         virtual std::int64_t insert(const LibraryEntry& entry) = 0;

         virtual bool removeByPath(const std::wstring& filePath) = 0;

         // Drop many rows in one transaction — what a reconciliation sweep
         // uses to prune every entry whose backing file has vanished at once.
         virtual bool removeByPaths(const std::vector<std::wstring>& filePaths) = 0;

         virtual bool setFavorite(const std::wstring& filePath, bool favorite) = 0;

         // Drop every entry (the settings dialog's "reset library"). The image
         // files on disk are untouched — only the rows go.
         virtual bool clearAll(void) = 0;
   };
}

#endif // TALAY_LIBRARY_STORE_ILIBRARYSTORE_HPP
