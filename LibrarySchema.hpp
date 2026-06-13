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

#ifndef TALAY_LIBRARY_STORE_LIBRARYSCHEMA_HPP
#define TALAY_LIBRARY_STORE_LIBRARYSCHEMA_HPP

namespace talay::Library::Schema
{
   // The single source of truth for the library_entry DDL: the store creates
   // the live table from it, and the transfer writes it into every dump so a
   // dump stays a self-contained script that replays anywhere.
   inline constexpr const wchar_t* k_createTableSql =
      L"CREATE TABLE IF NOT EXISTS library_entry ("
      L"id INTEGER PRIMARY KEY AUTOINCREMENT,"
      L"file_path TEXT NOT NULL UNIQUE,"
      L"file_name TEXT NOT NULL,"
      L"folder TEXT NOT NULL,"
      L"format TEXT NOT NULL,"
      L"file_size_bytes INTEGER NOT NULL DEFAULT 0,"
      L"width INTEGER NOT NULL DEFAULT 0,"
      L"height INTEGER NOT NULL DEFAULT 0,"
      L"frame_count INTEGER NOT NULL DEFAULT 1,"
      L"modified_utc INTEGER NOT NULL DEFAULT 0,"
      L"added_utc INTEGER NOT NULL DEFAULT 0,"
      L"is_favorite INTEGER NOT NULL DEFAULT 0)";
}

#endif // TALAY_LIBRARY_STORE_LIBRARYSCHEMA_HPP
