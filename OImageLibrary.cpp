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
#include "OImageLibrary.hpp"
#include "OSpecificationFactory.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <set>
#include <utility>

using namespace talay::Library;
using namespace talay::Library::Filter;
using anka::Core::Object::ObjectIdentity;


// -----------------------------------------------------------------------------
// OImageLibrary::Impl Definition
// -----------------------------------------------------------------------------
struct OImageLibrary::Impl
{
   ObjectIdentity m_identity {L"ImageLibraryObject"};

   std::unique_ptr<Store::ILibraryStore> m_store;
   std::vector<LibraryEntry>             m_entries;
   LibraryChangedCallback                m_onChanged;

   explicit Impl(std::unique_ptr<Store::ILibraryStore> store) :
      m_store {std::move(store)}
   {}

   bool open(const std::wstring& databasePath)
   {
      if (!m_store->open(databasePath))
         return false;

      m_entries = m_store->loadAll();
      return true;
   }

   void notifyChanged(void) const
   {
      if (m_onChanged)
         m_onChanged();
   }

   std::vector<LibraryEntry>::iterator findByPath(const std::wstring& filePath)
   {
      return std::find_if(m_entries.begin(), m_entries.end(),
                          [&filePath](const LibraryEntry& entry) { return entry.filePath == filePath; });
   }

   bool contains(const std::wstring& filePath) const
   {
      return std::any_of(m_entries.begin(), m_entries.end(),
                         [&filePath](const LibraryEntry& entry) { return entry.filePath == filePath; });
   }

   bool isFavorite(const std::wstring& filePath) const
   {
      return std::any_of(m_entries.begin(), m_entries.end(),
                         [&filePath](const LibraryEntry& entry)
                         { return entry.isFavorite && entry.filePath == filePath; });
   }

   // Adding an already-present path is a successful no-op, so the caller's
   // toggle button never needs to know whether the row already existed.
   bool add(const LibraryEntry& entry)
   {
      if (contains(entry.filePath))
         return true;

      LibraryEntry persisted {entry};
      persisted.id = m_store->insert(entry);
      if (persisted.id == 0)
         return false;

      m_entries.push_back(std::move(persisted));
      notifyChanged();
      return true;
   }

   bool remove(const std::wstring& filePath)
   {
      const auto entry = findByPath(filePath);
      if (entry == m_entries.end())
         return true;

      if (!m_store->removeByPath(filePath))
         return false;

      m_entries.erase(entry);
      notifyChanged();
      return true;
   }

   bool setFavorite(const std::wstring& filePath, bool favorite)
   {
      const auto entry = findByPath(filePath);
      if (entry == m_entries.end() || !m_store->setFavorite(filePath, favorite))
         return false;

      entry->isFavorite = favorite;
      notifyChanged();
      return true;
   }

   // The whole batch persists and erases in one go, then a single notify lets
   // the views re-query once — an empty list is a no-op success.
   bool removePaths(const std::vector<std::wstring>& filePaths)
   {
      if (filePaths.empty())
         return true;

      if (!m_store->removeByPaths(filePaths))
         return false;

      eraseEntries(filePaths);
      notifyChanged();
      return true;
   }

   void eraseEntries(const std::vector<std::wstring>& filePaths)
   {
      const std::set<std::wstring> dead {filePaths.begin(), filePaths.end()};

      std::erase_if(m_entries, [&dead](const LibraryEntry& entry)
                    { return dead.contains(entry.filePath); });
   }

   std::vector<std::wstring> allPaths(void) const
   {
      std::vector<std::wstring> paths;
      paths.reserve(m_entries.size());

      for (const LibraryEntry& entry : m_entries)
         paths.push_back(entry.filePath);

      return paths;
   }

   // An already-empty library clears successfully without touching the store.
   bool clear(void)
   {
      if (m_entries.empty())
         return true;

      if (!m_store->clearAll())
         return false;

      m_entries.clear();
      notifyChanged();
      return true;
   }

   std::vector<LibraryEntry> query(const LibraryFilter& filter) const
   {
      const auto specification = OSpecificationFactory::fromFilter(filter);

      std::vector<LibraryEntry> matches;
      std::copy_if(m_entries.begin(), m_entries.end(), std::back_inserter(matches),
                   [&specification](const LibraryEntry& entry)
                   { return specification->isSatisfiedBy(entry); });
      return matches;
   }

   // std::set gives distinctness and ordering in one pass.
   std::vector<std::wstring> distinctValues(std::wstring LibraryEntry::* field) const
   {
      std::set<std::wstring> values;
      for (const auto& entry : m_entries)
         values.insert(entry.*field);

      return {values.begin(), values.end()};
   }
};

// -----------------------------------------------------------------------------
// OImageLibrary Implementation
// -----------------------------------------------------------------------------
OImageLibrary::OImageLibrary(std::unique_ptr<Store::ILibraryStore> store) :
   m_pImpl {std::make_unique<Impl>(std::move(store))}
{}
// -----------------------------------------------------------------------------
OImageLibrary::~OImageLibrary() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OImageLibrary::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OImageLibrary::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImageLibrary::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OImageLibrary::open(const std::wstring& databasePath)
{
   return m_pImpl->open(databasePath);
}
// -----------------------------------------------------------------------------
bool OImageLibrary::add(const LibraryEntry& entry)
{
   return m_pImpl->add(entry);
}
// -----------------------------------------------------------------------------
bool OImageLibrary::remove(const std::wstring& filePath)
{
   return m_pImpl->remove(filePath);
}
// -----------------------------------------------------------------------------
bool OImageLibrary::setFavorite(const std::wstring& filePath, bool favorite)
{
   return m_pImpl->setFavorite(filePath, favorite);
}
// -----------------------------------------------------------------------------
bool OImageLibrary::removePaths(const std::vector<std::wstring>& filePaths)
{
   return m_pImpl->removePaths(filePaths);
}
// -----------------------------------------------------------------------------
bool OImageLibrary::clear(void)
{
   return m_pImpl->clear();
}
// -----------------------------------------------------------------------------
bool OImageLibrary::contains(const std::wstring& filePath) const
{
   return m_pImpl->contains(filePath);
}
// -----------------------------------------------------------------------------
bool OImageLibrary::isFavorite(const std::wstring& filePath) const
{
   return m_pImpl->isFavorite(filePath);
}
// -----------------------------------------------------------------------------
std::size_t OImageLibrary::entryCount(void) const
{
   return m_pImpl->m_entries.size();
}
// -----------------------------------------------------------------------------
std::vector<std::wstring> OImageLibrary::allPaths(void) const
{
   return m_pImpl->allPaths();
}
// -----------------------------------------------------------------------------
std::vector<LibraryEntry> OImageLibrary::query(const LibraryFilter& filter) const
{
   return m_pImpl->query(filter);
}
// -----------------------------------------------------------------------------
std::vector<std::wstring> OImageLibrary::availableFormats(void) const
{
   return m_pImpl->distinctValues(&LibraryEntry::format);
}
// -----------------------------------------------------------------------------
std::vector<std::wstring> OImageLibrary::availableFolders(void) const
{
   return m_pImpl->distinctValues(&LibraryEntry::folder);
}
// -----------------------------------------------------------------------------
void OImageLibrary::onChanged(LibraryChangedCallback callback)
{
   m_pImpl->m_onChanged = std::move(callback);
}
// -----------------------------------------------------------------------------
