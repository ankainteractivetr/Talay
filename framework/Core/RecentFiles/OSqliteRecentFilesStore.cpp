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
#include "OSqliteRecentFilesStore.hpp"
#include "RecentFilesSchema.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

using namespace anka::Core::RecentFiles;
using anka::Core::Database::IDatabase;
using anka::Core::Database::IStatement;
using anka::Core::Database::StepResult;
using anka::Core::Object::ObjectIdentity;


// The store's whole SQL vocabulary. The DDL lives in RecentFilesSchema.hpp;
// the row statements are the store's own. Recency = id order: see the schema
// header for why no timestamp comparison ever happens.
namespace
{
   constexpr const wchar_t* k_selectAllSql =
      L"SELECT file_path, opened_utc FROM recent_file ORDER BY id DESC";

   constexpr const wchar_t* k_insertSql =
      L"INSERT INTO recent_file (file_path, opened_utc) VALUES (?, ?)";

   constexpr const wchar_t* k_deleteByPathSql =
      L"DELETE FROM recent_file WHERE file_path = ?";

   constexpr const wchar_t* k_trimSql =
      L"DELETE FROM recent_file WHERE id NOT IN"
      L" (SELECT id FROM recent_file ORDER BY id DESC LIMIT ?)";
}


// -----------------------------------------------------------------------------
// OSqliteRecentFilesStore::Impl Definition
// -----------------------------------------------------------------------------
struct OSqliteRecentFilesStore::Impl
{
   ObjectIdentity m_identity {L"SqliteRecentFilesStoreObject"};

   std::unique_ptr<IDatabase> m_database;

   explicit Impl(std::unique_ptr<IDatabase> database) :
      m_database {std::move(database)}
   {}

   bool open(const std::wstring& databasePath)
   {
      return m_database->open(databasePath)
          && m_database->execute(Schema::k_createTableSql);
   }

   std::vector<RecentFile> loadAll(void)
   {
      std::vector<RecentFile> files;

      const auto statement = m_database->prepare(k_selectAllSql);
      if (!statement)
         return files;

      while (statement->step() == StepResult::Row)
         files.push_back(readFile(*statement));

      return files;
   }

   RecentFile readFile(const IStatement& statement)
   {
      return {.filePath  = statement.columnText(0),
              .openedUtc = statement.columnInt64(1)};
   }

   // Delete-then-insert on purpose: the fresh row gets a bigger id, which is
   // what moves the path to the front of the newest-first SELECT.
   bool touch(const RecentFile& file)
   {
      removeByPath(file.filePath);
      return insert(file);
   }

   bool insert(const RecentFile& file)
   {
      const auto statement = m_database->prepare(k_insertSql);
      if (!statement)
         return false;

      statement->bindText(1, file.filePath);
      statement->bindInt64(2, file.openedUtc);
      return statement->step() == StepResult::Done;
   }

   bool removeByPath(const std::wstring& filePath)
   {
      const auto statement = m_database->prepare(k_deleteByPathSql);
      if (!statement)
         return false;

      statement->bindText(1, filePath);
      return statement->step() == StepResult::Done;
   }

   bool trim(std::size_t capacity)
   {
      const auto statement = m_database->prepare(k_trimSql);
      if (!statement)
         return false;

      statement->bindInt64(1, static_cast<std::int64_t>(capacity));
      return statement->step() == StepResult::Done;
   }
};

// -----------------------------------------------------------------------------
// OSqliteRecentFilesStore Implementation
// -----------------------------------------------------------------------------
OSqliteRecentFilesStore::OSqliteRecentFilesStore(std::unique_ptr<IDatabase> database) :
   m_pImpl {std::make_unique<Impl>(std::move(database))}
{}
// -----------------------------------------------------------------------------
OSqliteRecentFilesStore::~OSqliteRecentFilesStore() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OSqliteRecentFilesStore::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OSqliteRecentFilesStore::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSqliteRecentFilesStore::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OSqliteRecentFilesStore::open(const std::wstring& databasePath)
{
   return m_pImpl->open(databasePath);
}
// -----------------------------------------------------------------------------
std::vector<RecentFile> OSqliteRecentFilesStore::loadAll(void)
{
   return m_pImpl->loadAll();
}
// -----------------------------------------------------------------------------
bool OSqliteRecentFilesStore::touch(const RecentFile& file)
{
   return m_pImpl->touch(file);
}
// -----------------------------------------------------------------------------
bool OSqliteRecentFilesStore::removeByPath(const std::wstring& filePath)
{
   return m_pImpl->removeByPath(filePath);
}
// -----------------------------------------------------------------------------
bool OSqliteRecentFilesStore::trim(std::size_t capacity)
{
   return m_pImpl->trim(capacity);
}
// -----------------------------------------------------------------------------
