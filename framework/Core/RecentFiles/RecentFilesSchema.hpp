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

#ifndef ANKA_CORE_RECENTFILES_RECENTFILESSCHEMA_HPP
#define ANKA_CORE_RECENTFILES_RECENTFILESSCHEMA_HPP

namespace anka::Core::RecentFiles::Schema
{
   // The single source of truth for the recent_file DDL. The autoincrement id
   // doubles as the recency order: a touched path is deleted and re-inserted,
   // so a bigger id always means "opened later" — no timestamp ties to break.
   inline constexpr const wchar_t* k_createTableSql =
      L"CREATE TABLE IF NOT EXISTS recent_file ("
      L"id INTEGER PRIMARY KEY AUTOINCREMENT,"
      L"file_path TEXT NOT NULL UNIQUE,"
      L"opened_utc INTEGER NOT NULL DEFAULT 0)";
}

#endif // ANKA_CORE_RECENTFILES_RECENTFILESSCHEMA_HPP
