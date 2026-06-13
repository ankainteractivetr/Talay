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

#ifndef ANKA_CORE_RECENTFILES_OSQLITERECENTFILESSTORE_HPP
#define ANKA_CORE_RECENTFILES_OSQLITERECENTFILESSTORE_HPP

#include "IRecentFilesStore.hpp"
#include "framework/Core/Database/IDatabase.hpp"

#include <memory>

namespace anka::Core::RecentFiles
{
   /*
   *
   * Concrete SQLite Recent Files Store Class
   *
   * Persists the recently-opened list in a single recent_file table. The
   * engine arrives through the constructor as an anka IDatabase (Dependency
   * Inversion), so this class is reduced to what is truly the store's: the
   * recent_file SQL statements and the RecentFile <-> row mapping. Recency
   * rides on the autoincrement id — touch() deletes and re-inserts the row,
   * so newest-first is simply ORDER BY id DESC. A database that failed to
   * open degrades gracefully — every operation no-ops and the app runs
   * without recents. One instance per database file — the tracker owns it
   * (not a singleton).
   *
   */
   class OSqliteRecentFilesStore final : public IRecentFilesStore
   {
      public:

         explicit OSqliteRecentFilesStore(std::unique_ptr<anka::Core::Database::IDatabase> database);

         OSqliteRecentFilesStore(const OSqliteRecentFilesStore& store) = delete;
         OSqliteRecentFilesStore(OSqliteRecentFilesStore&& store) noexcept = delete;

         ~OSqliteRecentFilesStore() noexcept override;

         OSqliteRecentFilesStore& operator=(const OSqliteRecentFilesStore& store) = delete;
         OSqliteRecentFilesStore& operator=(OSqliteRecentFilesStore&& store) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IRecentFilesStore overrides
         //

         bool open(const std::wstring& databasePath) override;

         std::vector<RecentFile> loadAll(void) override;

         bool touch(const RecentFile& file) override;

         bool removeByPath(const std::wstring& filePath) override;

         bool trim(std::size_t capacity) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_CORE_RECENTFILES_OSQLITERECENTFILESSTORE_HPP
