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


#include "pch.h"
#include "OSqliteLibraryStore.hpp"
#include "LibrarySchema.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using namespace talay::Library;
using namespace talay::Library::Store;
using anka::Core::Database::IDatabase;
using anka::Core::Database::IStatement;
using anka::Core::Database::StepResult;
using anka::Core::Object::ObjectIdentity;


// The store's whole SQL vocabulary. The DDL lives in LibrarySchema.hpp because
// the dump transfer shares it; the row statements are the store's own.
namespace
{
   constexpr const wchar_t* k_selectAllSql =
      L"SELECT id, file_path, file_name, folder, format, file_size_bytes,"
      L" width, height, frame_count, modified_utc, added_utc, is_favorite"
      L" FROM library_entry";

   constexpr const wchar_t* k_insertSql =
      L"INSERT INTO library_entry (file_path, file_name, folder, format,"
      L" file_size_bytes, width, height, frame_count, modified_utc, added_utc, is_favorite)"
      L" VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

   constexpr const wchar_t* k_deleteByPathSql =
      L"DELETE FROM library_entry WHERE file_path = ?";

   constexpr const wchar_t* k_updateFavoriteSql =
      L"UPDATE library_entry SET is_favorite = ? WHERE file_path = ?";

   constexpr const wchar_t* k_deleteAllSql =
      L"DELETE FROM library_entry";
}


// -----------------------------------------------------------------------------
// OSqliteLibraryStore::Impl Definition
// -----------------------------------------------------------------------------
struct OSqliteLibraryStore::Impl
{
   ObjectIdentity m_identity {L"SqliteLibraryStoreObject"};

   std::unique_ptr<IDatabase> m_database;

   explicit Impl(std::unique_ptr<IDatabase> database) :
      m_database {std::move(database)}
   {}

   bool open(const std::wstring& databasePath)
   {
      return m_database->open(databasePath)
          && m_database->execute(Schema::k_createTableSql);
   }

   std::vector<LibraryEntry> loadAll(void)
   {
      std::vector<LibraryEntry> entries;

      const auto statement = m_database->prepare(k_selectAllSql);
      if (!statement)
         return entries;

      while (statement->step() == StepResult::Row)
         entries.push_back(readEntry(*statement));

      return entries;
   }

   LibraryEntry readEntry(const IStatement& statement)
   {
      LibraryEntry entry;
      readIdentity(statement, entry);
      readFacts(statement, entry);
      return entry;
   }

   void readIdentity(const IStatement& statement, LibraryEntry& entry)
   {
      entry.id       = statement.columnInt64(0);
      entry.filePath = statement.columnText(1);
      entry.fileName = statement.columnText(2);
      entry.folder   = statement.columnText(3);
      entry.format   = statement.columnText(4);
   }

   void readFacts(const IStatement& statement, LibraryEntry& entry)
   {
      entry.fileSizeBytes = static_cast<std::uint64_t>(statement.columnInt64(5));
      entry.width         = static_cast<std::uint32_t>(statement.columnInt64(6));
      entry.height        = static_cast<std::uint32_t>(statement.columnInt64(7));
      entry.frameCount    = static_cast<std::uint32_t>(statement.columnInt64(8));
      entry.modifiedUtc   = statement.columnInt64(9);
      entry.addedUtc      = statement.columnInt64(10);
      entry.isFavorite    = statement.columnInt64(11) != 0;
   }

   std::int64_t insert(const LibraryEntry& entry)
   {
      const auto statement = m_database->prepare(k_insertSql);
      if (!statement)
         return 0;

      bindIdentity(*statement, entry);
      bindFacts(*statement, entry);

      return statement->step() == StepResult::Done ? m_database->lastInsertId() : 0;
   }

   void bindIdentity(IStatement& statement, const LibraryEntry& entry)
   {
      statement.bindText(1, entry.filePath);
      statement.bindText(2, entry.fileName);
      statement.bindText(3, entry.folder);
      statement.bindText(4, entry.format);
   }

   void bindFacts(IStatement& statement, const LibraryEntry& entry)
   {
      statement.bindInt64(5, static_cast<std::int64_t>(entry.fileSizeBytes));
      statement.bindInt64(6, entry.width);
      statement.bindInt64(7, entry.height);
      statement.bindInt64(8, entry.frameCount);
      statement.bindInt64(9, entry.modifiedUtc);
      statement.bindInt64(10, entry.addedUtc);
      statement.bindInt64(11, entry.isFavorite ? 1 : 0);
   }

   bool removeByPath(const std::wstring& filePath)
   {
      const auto statement = m_database->prepare(k_deleteByPathSql);
      if (!statement)
         return false;

      statement->bindText(1, filePath);
      return statement->step() == StepResult::Done;
   }

   // One transaction for the whole batch: a sweep that prunes many dead rows
   // pays a single commit, and a mid-batch failure rolls the lot back.
   bool removeByPaths(const std::vector<std::wstring>& filePaths)
   {
      if (filePaths.empty())
         return true;

      if (!m_database->execute(L"BEGIN IMMEDIATE"))
         return false;

      return commitRemovals(filePaths);
   }

   bool commitRemovals(const std::vector<std::wstring>& filePaths)
   {
      for (const std::wstring& path : filePaths)
         if (!removeByPath(path)) {
            m_database->execute(L"ROLLBACK");
            return false;
         }

      return m_database->execute(L"COMMIT");
   }

   bool setFavorite(const std::wstring& filePath, bool favorite)
   {
      const auto statement = m_database->prepare(k_updateFavoriteSql);
      if (!statement)
         return false;

      statement->bindInt64(1, favorite ? 1 : 0);
      statement->bindText(2, filePath);
      return statement->step() == StepResult::Done;
   }

   bool clearAll(void)
   {
      return m_database->execute(k_deleteAllSql);
   }
};

// -----------------------------------------------------------------------------
// OSqliteLibraryStore Implementation
// -----------------------------------------------------------------------------
OSqliteLibraryStore::OSqliteLibraryStore(std::unique_ptr<IDatabase> database) :
   m_pImpl {std::make_unique<Impl>(std::move(database))}
{}
// -----------------------------------------------------------------------------
OSqliteLibraryStore::~OSqliteLibraryStore() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OSqliteLibraryStore::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OSqliteLibraryStore::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSqliteLibraryStore::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OSqliteLibraryStore::open(const std::wstring& databasePath)
{
   return m_pImpl->open(databasePath);
}
// -----------------------------------------------------------------------------
std::vector<LibraryEntry> OSqliteLibraryStore::loadAll(void)
{
   return m_pImpl->loadAll();
}
// -----------------------------------------------------------------------------
std::int64_t OSqliteLibraryStore::insert(const LibraryEntry& entry)
{
   return m_pImpl->insert(entry);
}
// -----------------------------------------------------------------------------
bool OSqliteLibraryStore::removeByPath(const std::wstring& filePath)
{
   return m_pImpl->removeByPath(filePath);
}
// -----------------------------------------------------------------------------
bool OSqliteLibraryStore::removeByPaths(const std::vector<std::wstring>& filePaths)
{
   return m_pImpl->removeByPaths(filePaths);
}
// -----------------------------------------------------------------------------
bool OSqliteLibraryStore::setFavorite(const std::wstring& filePath, bool favorite)
{
   return m_pImpl->setFavorite(filePath, favorite);
}
// -----------------------------------------------------------------------------
bool OSqliteLibraryStore::clearAll(void)
{
   return m_pImpl->clearAll();
}
// -----------------------------------------------------------------------------
