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
#include "ORecentFilesTracker.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/System/Time/UnixTime.hpp"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

using namespace anka::Core::RecentFiles;
using anka::Core::Object::ObjectIdentity;


// The file-existence default behind the injectable FileProbe seam; the
// UtcClock seam defaults to the shared system clock (see the constructor).
namespace
{
   bool fileExists(const std::wstring& filePath)
   {
      return GetFileAttributesW(filePath.c_str()) != INVALID_FILE_ATTRIBUTES;
   }
}


// -----------------------------------------------------------------------------
// ORecentFilesTracker::Impl Definition
// -----------------------------------------------------------------------------
struct ORecentFilesTracker::Impl
{
   ObjectIdentity m_identity {L"RecentFilesTrackerObject"};

   std::unique_ptr<IRecentFilesStore> m_store;
   std::size_t m_capacity;

   FileProbe m_probe;
   UtcClock  m_clock;

   // Mirrors the store, most recent first — every read answers from here.
   std::vector<RecentFile> m_files;

   RecentFilesChangedCallback m_onChanged;

   Impl(std::unique_ptr<IRecentFilesStore> store, std::size_t capacity,
        FileProbe probe, UtcClock clock) :
      m_store {std::move(store)},
      m_capacity {capacity},
      m_probe {probe ? std::move(probe) : fileExists},
      m_clock {clock ? std::move(clock) : anka::System::Time::currentUnixSeconds}
   {}

   bool open(const std::wstring& databasePath)
   {
      if (!m_store->open(databasePath))
         return false;

      m_files = m_store->loadAll();
      pruneMissing();
      return true;
   }

   // The launch-time existence sweep: entries whose file vanished since the
   // last run leave both the mirror and the store, quietly.
   void pruneMissing(void)
   {
      const auto missing = [this](const RecentFile& file) { return !m_probe(file.filePath); };

      for (const auto& file : m_files | std::views::filter(missing))
         m_store->removeByPath(file.filePath);

      std::erase_if(m_files, missing);
   }

   void record(const std::wstring& filePath)
   {
      const RecentFile file {.filePath = filePath, .openedUtc = m_clock()};

      moveToFront(file);
      m_store->touch(file);
      m_store->trim(m_capacity);
      notify();
   }

   void moveToFront(const RecentFile& file)
   {
      std::erase_if(m_files, matches(file.filePath));
      m_files.insert(m_files.begin(), file);

      if (m_files.size() > m_capacity)
         m_files.resize(m_capacity);
   }

   bool revalidate(const std::wstring& filePath)
   {
      if (m_probe(filePath))
         return true;

      discard(filePath);
      return false;
   }

   bool discard(const std::wstring& filePath)
   {
      const auto dropped = std::erase_if(m_files, matches(filePath));

      m_store->removeByPath(filePath);
      notify();
      return dropped > 0;
   }

   auto matches(const std::wstring& filePath) const
   {
      return [&filePath](const RecentFile& file) { return file.filePath == filePath; };
   }

   std::vector<std::wstring> files(void) const
   {
      std::vector<std::wstring> paths;
      paths.reserve(m_files.size());

      std::ranges::transform(m_files, std::back_inserter(paths), &RecentFile::filePath);
      return paths;
   }

   void notify(void)
   {
      if (m_onChanged)
         m_onChanged();
   }
};

// -----------------------------------------------------------------------------
// ORecentFilesTracker Implementation
// -----------------------------------------------------------------------------
ORecentFilesTracker::ORecentFilesTracker(std::unique_ptr<IRecentFilesStore> store,
                                         std::size_t capacity,
                                         FileProbe probe, UtcClock clock) :
   m_pImpl {std::make_unique<Impl>(std::move(store), capacity, std::move(probe), std::move(clock))}
{}
// -----------------------------------------------------------------------------
ORecentFilesTracker::~ORecentFilesTracker() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& ORecentFilesTracker::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void ORecentFilesTracker::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t ORecentFilesTracker::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool ORecentFilesTracker::open(const std::wstring& databasePath)
{
   return m_pImpl->open(databasePath);
}
// -----------------------------------------------------------------------------
void ORecentFilesTracker::record(const std::wstring& filePath)
{
   m_pImpl->record(filePath);
}
// -----------------------------------------------------------------------------
bool ORecentFilesTracker::revalidate(const std::wstring& filePath)
{
   return m_pImpl->revalidate(filePath);
}
// -----------------------------------------------------------------------------
bool ORecentFilesTracker::discard(const std::wstring& filePath)
{
   return m_pImpl->discard(filePath);
}
// -----------------------------------------------------------------------------
std::vector<std::wstring> ORecentFilesTracker::files(void) const
{
   return m_pImpl->files();
}
// -----------------------------------------------------------------------------
std::size_t ORecentFilesTracker::capacity(void) const
{
   return m_pImpl->m_capacity;
}
// -----------------------------------------------------------------------------
void ORecentFilesTracker::onChanged(RecentFilesChangedCallback callback)
{
   m_pImpl->m_onChanged = std::move(callback);
}
// -----------------------------------------------------------------------------
