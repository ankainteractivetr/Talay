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
#include "OLibraryTransfer.hpp"
#include "LibrarySchema.hpp"
#include "LibraryFilter.hpp"
#include "framework/Core/IO/File/OTextFile.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstdint>
#include <filesystem>
#include <format>
#include <string>
#include <utility>
#include <vector>

using namespace talay::Library;
using namespace talay::Library::Transfer;
using anka::Core::Database::IDatabase;
using anka::Core::Database::IStatement;
using anka::Core::Database::StepResult;
using anka::Core::File::OTextFile;
using anka::Core::Object::ObjectIdentity;


namespace
{
   // The dump reads its rows back in exactly the column order its own INSERTs
   // wrote. Ids are deliberately left out of the dump, so the receiving table
   // mints fresh ones on replay and two libraries can merge without clashing.
   constexpr const wchar_t* k_selectRowsSql =
      L"SELECT file_path, file_name, folder, format, file_size_bytes,"
      L" width, height, frame_count, modified_utc, added_utc, is_favorite"
      L" FROM library_entry";

   // SQL text literal: wrapped in single quotes with inner quotes doubled —
   // the only escaping SQL string syntax needs.
   std::wstring quoted(const std::wstring& text)
   {
      std::wstring literal;
      literal.reserve(text.size() + 2);

      literal += L'\'';
      for (const wchar_t character : text) {
         literal += character;
         if (character == L'\'')
            literal += character;
      }
      literal += L'\'';

      return literal;
   }

   bool fileExists(const std::wstring& path)
   {
      const DWORD attributes {GetFileAttributesW(path.c_str())};
      return attributes != INVALID_FILE_ATTRIBUTES
         && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
   }
}


// -----------------------------------------------------------------------------
// OLibraryTransfer::Impl Definition
// -----------------------------------------------------------------------------
struct OLibraryTransfer::Impl
{
   ObjectIdentity m_identity {L"LibraryTransferObject"};

   IImageLibrary&  m_library;
   DatabaseFactory m_makeDatabase;

   Impl(IImageLibrary& library, DatabaseFactory makeDatabase) :
      m_library      {library},
      m_makeDatabase {std::move(makeDatabase)}
   {}

   //
   // Export — repository entries to one self-contained SQL script
   //

   bool exportTo(const std::wstring& dumpFilePath)
   {
      return writeDump(dumpFilePath, buildScript(m_library.query(LibraryFilter {})));
   }

   std::wstring buildScript(const std::vector<LibraryEntry>& entries)
   {
      std::wstring script {Schema::k_createTableSql};
      script += L";\nBEGIN TRANSACTION;\n";

      for (const auto& entry : entries)
         script += insertStatement(entry);

      script += L"COMMIT;\n";
      return script;
   }

   std::wstring insertStatement(const LibraryEntry& entry)
   {
      return std::format(
         L"INSERT INTO library_entry (file_path, file_name, folder, format, file_size_bytes,"
         L" width, height, frame_count, modified_utc, added_utc, is_favorite)"
         L" VALUES ({}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});\n",
         quoted(entry.filePath), quoted(entry.fileName), quoted(entry.folder), quoted(entry.format),
         entry.fileSizeBytes, entry.width, entry.height, entry.frameCount,
         entry.modifiedUtc, entry.addedUtc, entry.isFavorite ? 1 : 0);
   }

   bool writeDump(const std::wstring& dumpFilePath, const std::wstring& script)
   {
      const std::filesystem::path path {dumpFilePath};
      OTextFile file {L"LibraryDumpFile", path.filename().wstring(), path.parent_path()};
      if (!file.open() || !file.truncate())
         return false;

      const bool written {file.appendText(script)};
      file.close();
      return written;
   }

   //
   // Import — replay a script and merge its rows into the library
   //

   TransferSummary importFrom(const std::wstring& dumpFilePath)
   {
      TransferSummary summary;

      const auto database = replayDump(readDump(dumpFilePath));
      if (!database)
         return summary;

      mergeRows(*database, summary);
      summary.succeeded = true;
      return summary;
   }

   std::wstring readDump(const std::wstring& dumpFilePath)
   {
      const std::filesystem::path path {dumpFilePath};
      OTextFile file {L"LibraryDumpFile", path.filename().wstring(), path.parent_path()};
      if (!file.open())
         return {};

      std::wstring script {file.readAllText()};
      file.close();
      return script;
   }

   // The script replays into a scratch in-memory database, so broken (or
   // hostile) dump text can never touch the live library file.
   std::unique_ptr<IDatabase> replayDump(const std::wstring& script)
   {
      if (script.empty())
         return nullptr;

      auto database = m_makeDatabase();
      if (!database || !database->open(L":memory:") || !database->runScript(script))
         return nullptr;

      return database;
   }

   void mergeRows(IDatabase& database, TransferSummary& summary)
   {
      const auto statement = database.prepare(k_selectRowsSql);
      if (!statement)
         return;

      while (statement->step() == StepResult::Row)
         mergeEntry(readEntry(*statement), summary);
   }

   // The merge policy: a path already in the library is skipped, a row whose
   // image file is gone from this machine is skipped, everything else is
   // added — which persists it through the live store and keeps its favorite
   // mark and original timestamps.
   void mergeEntry(const LibraryEntry& entry, TransferSummary& summary)
   {
      if (m_library.contains(entry.filePath))
         ++summary.skippedExisting;
      else if (!fileExists(entry.filePath))
         ++summary.skippedMissing;
      else if (m_library.add(entry))
         ++summary.imported;
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
      entry.filePath = statement.columnText(0);
      entry.fileName = statement.columnText(1);
      entry.folder   = statement.columnText(2);
      entry.format   = statement.columnText(3);
   }

   void readFacts(const IStatement& statement, LibraryEntry& entry)
   {
      entry.fileSizeBytes = static_cast<std::uint64_t>(statement.columnInt64(4));
      entry.width         = static_cast<std::uint32_t>(statement.columnInt64(5));
      entry.height        = static_cast<std::uint32_t>(statement.columnInt64(6));
      entry.frameCount    = static_cast<std::uint32_t>(statement.columnInt64(7));
      entry.modifiedUtc   = statement.columnInt64(8);
      entry.addedUtc      = statement.columnInt64(9);
      entry.isFavorite    = statement.columnInt64(10) != 0;
   }
};

// -----------------------------------------------------------------------------
// OLibraryTransfer Implementation
// -----------------------------------------------------------------------------
OLibraryTransfer::OLibraryTransfer(IImageLibrary& library, DatabaseFactory makeDatabase) :
   m_pImpl {std::make_unique<Impl>(library, std::move(makeDatabase))}
{}
// -----------------------------------------------------------------------------
OLibraryTransfer::~OLibraryTransfer() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OLibraryTransfer::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OLibraryTransfer::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OLibraryTransfer::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OLibraryTransfer::exportTo(const std::wstring& dumpFilePath)
{
   return m_pImpl->exportTo(dumpFilePath);
}
// -----------------------------------------------------------------------------
TransferSummary OLibraryTransfer::importFrom(const std::wstring& dumpFilePath)
{
   return m_pImpl->importFrom(dumpFilePath);
}
// -----------------------------------------------------------------------------
