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

#ifndef ANKA_CORE_DATABASE_IDATABASE_HPP
#define ANKA_CORE_DATABASE_IDATABASE_HPP

#include "IStatement.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <cstdint>
#include <memory>
#include <string>

namespace anka::Core::Database
{
   /*
   * Abstract Database Interface
   *
   * One connection to a SQL database: open a file (or L":memory:" for a
   * scratch database), run one-shot statements, replay multi-statement
   * scripts, and prepare parameterized statements (Factory Method — the
   * concrete statement type never leaks past this seam). Every call is
   * throw-free: a database that failed to open reports failure and yields
   * nothing, so consumers degrade gracefully instead of handling exceptions.
   * Consumers own this seam (Dependency Inversion); the SQL engine behind it
   * stays swappable.
   *
   */
   class IDatabase : public anka::Core::Object::IObject
   {
      public:

         virtual ~IDatabase() noexcept override = default;

         virtual bool open(const std::wstring& databasePath) = 0;
         virtual void close(void) = 0;

         // Run one self-contained statement (DDL, DELETE, pragma, ...).
         virtual bool execute(const std::wstring& sql) = 0;

         // Replay every ';'-terminated statement in 'script' in order,
         // stopping at the first failure — how a dump file is imported.
         virtual bool runScript(const std::wstring& script) = 0;

         // Prepare a (possibly parameterized) statement; null on failure. The
         // statement must not outlive this database.
         virtual std::unique_ptr<IStatement> prepare(const std::wstring& sql) = 0;

         // Row id handed to the most recent successful INSERT, 0 if none.
         virtual std::int64_t lastInsertId(void) const = 0;
   };
}

#endif // ANKA_CORE_DATABASE_IDATABASE_HPP
