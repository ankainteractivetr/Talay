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

#ifndef ANKA_CORE_RECENTFILES_ORECENTFILESTRACKER_HPP
#define ANKA_CORE_RECENTFILES_ORECENTFILESTRACKER_HPP

#include "IRecentFilesTracker.hpp"
#include "IRecentFilesStore.hpp"

#include <cstdint>
#include <memory>

namespace anka::Core::RecentFiles
{
   // The tracker's two environment touch-points, injectable so tests can
   // fake the file system and the clock (Strategy). Left empty at the call
   // site, the Windows API defaults step in.
   using FileProbe = std::function<bool(const std::wstring& /*filePath*/)>;
   using UtcClock  = std::function<std::int64_t(void)>;

   /*
   *
   * Concrete Recent Files Tracker Class
   *
   * Answers reads from an in-memory newest-first list and writes every
   * mutation through the injected store (Dependency Inversion), trimming
   * both to the fixed capacity. Existence checks go through the injected
   * FileProbe and timestamps through the injected UtcClock, so the class
   * holds policy only — no file system, no SQL, no clock of its own. One
   * instance per program window owns it (not a singleton).
   *
   */
   class ORecentFilesTracker final : public IRecentFilesTracker
   {
      public:

         ORecentFilesTracker(std::unique_ptr<IRecentFilesStore> store,
                             std::size_t capacity,
                             FileProbe probe = {},
                             UtcClock clock = {});

         ORecentFilesTracker(const ORecentFilesTracker& tracker) = delete;
         ORecentFilesTracker(ORecentFilesTracker&& tracker) noexcept = delete;

         ~ORecentFilesTracker() noexcept override;

         ORecentFilesTracker& operator=(const ORecentFilesTracker& tracker) = delete;
         ORecentFilesTracker& operator=(ORecentFilesTracker&& tracker) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IRecentFilesTracker overrides
         //

         bool open(const std::wstring& databasePath) override;

         void record(const std::wstring& filePath) override;

         bool revalidate(const std::wstring& filePath) override;

         bool discard(const std::wstring& filePath) override;

         std::vector<std::wstring> files(void) const override;

         std::size_t capacity(void) const override;

         void onChanged(RecentFilesChangedCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_CORE_RECENTFILES_ORECENTFILESTRACKER_HPP
