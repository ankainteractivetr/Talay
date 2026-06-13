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

#ifndef ANKA_CORE_DATABASE_ISTATEMENT_HPP
#define ANKA_CORE_DATABASE_ISTATEMENT_HPP

#include <cstdint>
#include <string>

namespace anka::Core::Database
{
   // What one step() of a prepared statement produced: a result row ready to
   // read, successful completion, or failure.
   enum class StepResult
   {
      Row,
      Done,
      Error
   };

   /*
   * Abstract Database Statement Interface
   *
   * One prepared SQL statement: bind parameters by 1-based index, step through
   * execution, and read the current row's columns by 0-based index. Statements
   * are throwaway per-query values minted by IDatabase::prepare (Factory
   * Method); they carry no identity, so they deliberately stay outside the
   * IObject hierarchy. The concrete statement releases its engine resources on
   * destruction (RAII) and must not outlive the database that prepared it.
   *
   */
   class IStatement
   {
      public:

         virtual ~IStatement() noexcept = default;

         virtual bool bindText(int index, const std::wstring& text) = 0;
         virtual bool bindInt64(int index, std::int64_t value) = 0;

         virtual StepResult step(void) = 0;

         virtual std::wstring columnText(int column) const = 0;
         virtual std::int64_t columnInt64(int column) const = 0;
   };
}

#endif // ANKA_CORE_DATABASE_ISTATEMENT_HPP
