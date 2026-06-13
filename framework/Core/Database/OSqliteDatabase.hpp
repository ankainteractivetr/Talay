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

#ifndef ANKA_CORE_DATABASE_OSQLITEDATABASE_HPP
#define ANKA_CORE_DATABASE_OSQLITEDATABASE_HPP

#include "IDatabase.hpp"

#include <memory>

namespace anka::Core::Database
{
   /*
   *
   * Concrete SQLite Database Class
   *
   * IDatabase over the winsqlite3.dll that ships inside Windows (System32) —
   * real SQL with zero vendored code. The DLL is bound at runtime via
   * LoadLibrary / GetProcAddress (pure Windows API), so a machine without it
   * degrades gracefully: open() fails and every operation no-ops. All SQLite
   * types, function pointers and statement plumbing live behind the pimpl;
   * this header leaks no SQL. One instance per connection — the consumer owns
   * it (not a singleton).
   *
   */
   class OSqliteDatabase final : public IDatabase
   {
      public:

         OSqliteDatabase();

         OSqliteDatabase(const OSqliteDatabase& database) = delete;
         OSqliteDatabase(OSqliteDatabase&& database) noexcept = delete;

         ~OSqliteDatabase() noexcept override;

         OSqliteDatabase& operator=(const OSqliteDatabase& database) = delete;
         OSqliteDatabase& operator=(OSqliteDatabase&& database) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IDatabase overrides
         //

         bool open(const std::wstring& databasePath) override;
         void close(void) override;

         bool execute(const std::wstring& sql) override;

         bool runScript(const std::wstring& script) override;

         std::unique_ptr<IStatement> prepare(const std::wstring& sql) override;

         std::int64_t lastInsertId(void) const override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_CORE_DATABASE_OSQLITEDATABASE_HPP
