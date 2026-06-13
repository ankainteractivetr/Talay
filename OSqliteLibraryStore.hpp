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

#ifndef TALAY_LIBRARY_STORE_OSQLITELIBRARYSTORE_HPP
#define TALAY_LIBRARY_STORE_OSQLITELIBRARYSTORE_HPP

#include "ILibraryStore.hpp"
#include "framework/Core/Database/IDatabase.hpp"

#include <memory>

namespace talay::Library::Store
{
   /*
   *
   * Concrete SQLite Library Store Class
   *
   * Persists library entries in a single-table SQLite database. The engine
   * arrives through the constructor as an anka IDatabase (Dependency
   * Inversion), so this class is reduced to what is truly the store's: the
   * library_entry SQL statements and the LibraryEntry <-> row mapping. A
   * database that failed to open degrades gracefully — every operation
   * no-ops and the app runs without a library. One instance per database
   * file — the repository owns it (not a singleton).
   *
   */
   class OSqliteLibraryStore final : public ILibraryStore
   {
      public:

         explicit OSqliteLibraryStore(std::unique_ptr<anka::Core::Database::IDatabase> database);

         OSqliteLibraryStore(const OSqliteLibraryStore& store) = delete;
         OSqliteLibraryStore(OSqliteLibraryStore&& store) noexcept = delete;

         ~OSqliteLibraryStore() noexcept override;

         OSqliteLibraryStore& operator=(const OSqliteLibraryStore& store) = delete;
         OSqliteLibraryStore& operator=(OSqliteLibraryStore&& store) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // ILibraryStore overrides
         //

         bool open(const std::wstring& databasePath) override;

         std::vector<LibraryEntry> loadAll(void) override;

         std::int64_t insert(const LibraryEntry& entry) override;

         bool removeByPath(const std::wstring& filePath) override;

         bool removeByPaths(const std::vector<std::wstring>& filePaths) override;

         bool setFavorite(const std::wstring& filePath, bool favorite) override;

         bool clearAll(void) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_LIBRARY_STORE_OSQLITELIBRARYSTORE_HPP
