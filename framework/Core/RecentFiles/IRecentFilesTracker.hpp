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

#ifndef ANKA_CORE_RECENTFILES_IRECENTFILESTRACKER_HPP
#define ANKA_CORE_RECENTFILES_IRECENTFILESTRACKER_HPP

#include "framework/Core/Object/IObject.hpp"

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace anka::Core::RecentFiles
{
   // Fired after any mutation (record / discard), so views rebuild instead
   // of tracking individual changes.
   using RecentFilesChangedCallback = std::function<void(void)>;

   /*
   * Abstract Recent Files Tracker Interface (Repository + Facade)
   *
   * The one seam a program talks to about its recently-opened files: a
   * most-recently-used list with a fixed capacity, persisted across runs.
   * record() moves a path to the front (entering it if new, dropping the
   * oldest entry past capacity), open() loads what the last run remembered
   * and silently prunes files that no longer exist on disk, and
   * revalidate() re-checks one entry the moment the user picks it. How
   * entries persist (SQLite today) stays an implementation detail behind
   * this interface.
   *
   */
   class IRecentFilesTracker : public anka::Core::Object::IObject
   {
      public:

         virtual ~IRecentFilesTracker() noexcept override = default;

         // Open (creating on first run) the backing database, load the
         // remembered list and prune entries whose file is gone; false
         // leaves an empty, non-persisting list the app can still run with.
         virtual bool open(const std::wstring& databasePath) = 0;

         // Make 'filePath' the most recent entry.
         virtual void record(const std::wstring& filePath) = 0;

         // True while 'filePath' still exists on disk; a vanished file is
         // discarded from the list and reported with false.
         virtual bool revalidate(const std::wstring& filePath) = 0;

         // Forget 'filePath'; true when it was actually in the list.
         virtual bool discard(const std::wstring& filePath) = 0;

         // The remembered paths, most recent first — what a menu shows.
         virtual std::vector<std::wstring> files(void) const = 0;

         virtual std::size_t capacity(void) const = 0;

         virtual void onChanged(RecentFilesChangedCallback callback) = 0;
   };
}

#endif // ANKA_CORE_RECENTFILES_IRECENTFILESTRACKER_HPP
