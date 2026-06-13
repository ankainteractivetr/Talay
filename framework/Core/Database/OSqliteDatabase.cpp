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
#include "OSqliteDatabase.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstdint>
#include <memory>
#include <string>

using namespace anka::Core::Database;
using anka::Core::Object::ObjectIdentity;


// The slice of the SQLite C surface this adapter uses, declared by hand because
// Windows ships winsqlite3.dll without a header. Everything stays file-local:
// no SQLite name leaks past this translation unit.
namespace
{
   struct sqlite3;
   struct sqlite3_stmt;

   constexpr int k_sqliteOk   {0};
   constexpr int k_sqliteRow  {100};
   constexpr int k_sqliteDone {101};

   // SQLITE_TRANSIENT: tells SQLite to take its own copy of bound text.
   using TextDestructor = void (*)(void*);
   const TextDestructor k_copyText {reinterpret_cast<TextDestructor>(static_cast<std::intptr_t>(-1))};

   using OpenFn        = int (*)(const void* fileNameUtf16, sqlite3** database);
   using CloseFn       = int (*)(sqlite3* database);
   using PrepareFn     = int (*)(sqlite3* database, const void* sqlUtf16, int bytes,
                                 sqlite3_stmt** statement, const void** tail);
   using StepFn        = int (*)(sqlite3_stmt* statement);
   using FinalizeFn    = int (*)(sqlite3_stmt* statement);
   using BindTextFn    = int (*)(sqlite3_stmt* statement, int index, const void* textUtf16,
                                 int bytes, TextDestructor destructor);
   using BindInt64Fn   = int (*)(sqlite3_stmt* statement, int index, long long value);
   using ColumnTextFn  = const void* (*)(sqlite3_stmt* statement, int column);
   using ColumnInt64Fn = long long (*)(sqlite3_stmt* statement, int column);
   using LastRowIdFn   = long long (*)(sqlite3* database);

   struct SqliteApi
   {
      OpenFn        open16       {nullptr};
      CloseFn       close        {nullptr};
      PrepareFn     prepare16    {nullptr};
      StepFn        step         {nullptr};
      FinalizeFn    finalize     {nullptr};
      BindTextFn    bindText16   {nullptr};
      BindInt64Fn   bindInt64    {nullptr};
      ColumnTextFn  columnText16 {nullptr};
      ColumnInt64Fn columnInt64  {nullptr};
      LastRowIdFn   lastRowId    {nullptr};

      bool complete(void) const
      {
         return open16 && close && prepare16 && step && finalize
             && bindText16 && bindInt64 && columnText16 && columnInt64 && lastRowId;
      }
   };

   template <typename Function>
   Function procedure(HMODULE module, const char* name)
   {
      return reinterpret_cast<Function>(GetProcAddress(module, name));
   }

   void loadConnectionApi(HMODULE module, SqliteApi& api)
   {
      api.open16    = procedure<OpenFn>(module, "sqlite3_open16");
      api.close     = procedure<CloseFn>(module, "sqlite3_close");
      api.prepare16 = procedure<PrepareFn>(module, "sqlite3_prepare16_v2");
      api.lastRowId = procedure<LastRowIdFn>(module, "sqlite3_last_insert_rowid");
   }

   void loadStatementApi(HMODULE module, SqliteApi& api)
   {
      api.step         = procedure<StepFn>(module, "sqlite3_step");
      api.finalize     = procedure<FinalizeFn>(module, "sqlite3_finalize");
      api.bindText16   = procedure<BindTextFn>(module, "sqlite3_bind_text16");
      api.bindInt64    = procedure<BindInt64Fn>(module, "sqlite3_bind_int64");
      api.columnText16 = procedure<ColumnTextFn>(module, "sqlite3_column_text16");
      api.columnInt64  = procedure<ColumnInt64Fn>(module, "sqlite3_column_int64");
   }

   SqliteApi loadApi(HMODULE module)
   {
      SqliteApi api {};
      loadConnectionApi(module, api);
      loadStatementApi(module, api);
      return api;
   }

   // The concrete IStatement: RAII over one prepared sqlite3_stmt, finalized
   // in the destructor so no early return can leak statement memory. Borrows
   // the owning database's function table, so it must not outlive it — which
   // IDatabase::prepare's contract already demands.
   class OSqliteStatement final : public IStatement
   {
      public:

         OSqliteStatement(const SqliteApi& api, sqlite3_stmt* statement) :
            m_api       {api},
            m_statement {statement}
         {}

         OSqliteStatement(const OSqliteStatement& statement) = delete;
         OSqliteStatement& operator=(const OSqliteStatement& statement) = delete;

         ~OSqliteStatement() noexcept override
         {
            m_api.finalize(m_statement);
         }

         bool bindText(int index, const std::wstring& text) override
         {
            return m_api.bindText16(m_statement, index, text.c_str(), -1, k_copyText) == k_sqliteOk;
         }

         bool bindInt64(int index, std::int64_t value) override
         {
            return m_api.bindInt64(m_statement, index, value) == k_sqliteOk;
         }

         StepResult step(void) override
         {
            const int result {m_api.step(m_statement)};

            if (result == k_sqliteRow)
               return StepResult::Row;

            return (result == k_sqliteDone) ? StepResult::Done : StepResult::Error;
         }

         std::wstring columnText(int column) const override
         {
            const auto* text = static_cast<const wchar_t*>(m_api.columnText16(m_statement, column));
            return text ? text : L"";
         }

         std::int64_t columnInt64(int column) const override
         {
            return m_api.columnInt64(m_statement, column);
         }

      private:

         const SqliteApi& m_api;
         sqlite3_stmt*    m_statement;
   };
}


// -----------------------------------------------------------------------------
// OSqliteDatabase::Impl Definition
// -----------------------------------------------------------------------------
struct OSqliteDatabase::Impl
{
   ObjectIdentity m_identity {L"SqliteDatabaseObject"};

   HMODULE   m_module   {nullptr};
   SqliteApi m_api      {};
   sqlite3*  m_database {nullptr};

   ~Impl() noexcept
   {
      close();
      unloadModule();
   }

   void close(void)
   {
      if (m_database) {
         m_api.close(m_database);
         m_database = nullptr;
      }
   }

   void unloadModule(void)
   {
      if (m_module) {
         FreeLibrary(m_module);
         m_module = nullptr;
      }
   }

   // Bind the Windows-shipped SQLite once; LOAD_LIBRARY_SEARCH_SYSTEM32 pins
   // the lookup to System32 so no local DLL can shadow it.
   bool loadModule(void)
   {
      if (m_module)
         return m_api.complete();

      m_module = LoadLibraryExW(L"winsqlite3.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);
      if (!m_module)
         return false;

      m_api = loadApi(m_module);
      return m_api.complete();
   }

   bool open(const std::wstring& databasePath)
   {
      if (!loadModule())
         return false;

      close();
      if (m_api.open16(databasePath.c_str(), &m_database) != k_sqliteOk) {
         close();
         return false;
      }

      return true;
   }

   std::unique_ptr<IStatement> prepare(const std::wstring& sql)
   {
      if (!m_database)
         return nullptr;

      sqlite3_stmt* raw {nullptr};
      if (m_api.prepare16(m_database, sql.c_str(), -1, &raw, nullptr) != k_sqliteOk || !raw)
         return nullptr;

      return std::make_unique<OSqliteStatement>(m_api, raw);
   }

   bool execute(const std::wstring& sql)
   {
      const auto statement = prepare(sql);
      return statement && statement->step() == StepResult::Done;
   }

   // One statement starting at 'cursor': prepare it, run it to its first
   // result, and advance 'cursor' past it. Trailing whitespace after the last
   // ';' prepares to a null statement — reported as a clean end via
   // 'finished', not as an error.
   bool runStatementAt(const void*& cursor, bool& finished)
   {
      sqlite3_stmt* raw {nullptr};
      const void* tail {nullptr};
      if (m_api.prepare16(m_database, cursor, -1, &raw, &tail) != k_sqliteOk)
         return false;

      cursor   = tail;
      finished = (raw == nullptr);
      if (finished)
         return true;

      OSqliteStatement statement {m_api, raw};
      return statement.step() != StepResult::Error;
   }

   bool runScript(const std::wstring& script)
   {
      if (!m_database)
         return false;

      const void* cursor {script.c_str()};
      bool finished {false};
      while (!finished) {
         if (!runStatementAt(cursor, finished))
            return false;
      }

      return true;
   }

   std::int64_t lastInsertId(void) const
   {
      return m_database ? m_api.lastRowId(m_database) : 0;
   }
};

// -----------------------------------------------------------------------------
// OSqliteDatabase Implementation
// -----------------------------------------------------------------------------
OSqliteDatabase::OSqliteDatabase() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OSqliteDatabase::~OSqliteDatabase() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OSqliteDatabase::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OSqliteDatabase::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSqliteDatabase::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OSqliteDatabase::open(const std::wstring& databasePath)
{
   return m_pImpl->open(databasePath);
}
// -----------------------------------------------------------------------------
void OSqliteDatabase::close(void)
{
   m_pImpl->close();
}
// -----------------------------------------------------------------------------
bool OSqliteDatabase::execute(const std::wstring& sql)
{
   return m_pImpl->execute(sql);
}
// -----------------------------------------------------------------------------
bool OSqliteDatabase::runScript(const std::wstring& script)
{
   return m_pImpl->runScript(script);
}
// -----------------------------------------------------------------------------
std::unique_ptr<IStatement> OSqliteDatabase::prepare(const std::wstring& sql)
{
   return m_pImpl->prepare(sql);
}
// -----------------------------------------------------------------------------
std::int64_t OSqliteDatabase::lastInsertId(void) const
{
   return m_pImpl->lastInsertId();
}
// -----------------------------------------------------------------------------
