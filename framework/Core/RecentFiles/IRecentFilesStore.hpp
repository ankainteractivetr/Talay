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

#ifndef ANKA_CORE_RECENTFILES_IRECENTFILESSTORE_HPP
#define ANKA_CORE_RECENTFILES_IRECENTFILESSTORE_HPP

#include "RecentFile.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace anka::Core::RecentFiles
{
   /*
   * Abstract Recent Files Store Interface
   *
   * Pure persistence for the recently-opened list: open a database file
   * once, load the remembered opens newest-first, and write through the
   * three mutations recency needs — touch (move a path to the front),
   * remove, trim. Policy (capacity, which files still exist) lives in the
   * tracker above it (Dependency Inversion: the tracker owns this seam, so
   * the SQL engine behind it can be swapped without touching anything else).
   *
   * Every call is throw-free: a store that failed to open simply reports
   * failure / yields nothing, and the app keeps running without recents.
   *
   */
   class IRecentFilesStore : public anka::Core::Object::IObject
   {
      public:

         virtual ~IRecentFilesStore() noexcept override = default;

         // Open (creating on first run) the database at 'databasePath'.
         virtual bool open(const std::wstring& databasePath) = 0;

         // Every remembered open, most recent first.
         virtual std::vector<RecentFile> loadAll(void) = 0;

         // Make 'file' the most recent entry, whether or not it was already
         // remembered.
         virtual bool touch(const RecentFile& file) = 0;

         virtual bool removeByPath(const std::wstring& filePath) = 0;

         // Drop everything beyond the 'capacity' most recent entries.
         virtual bool trim(std::size_t capacity) = 0;
   };
}

#endif // ANKA_CORE_RECENTFILES_IRECENTFILESSTORE_HPP
