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

#ifndef TALAY_LIBRARY_OIMAGELIBRARY_HPP
#define TALAY_LIBRARY_OIMAGELIBRARY_HPP

#include "IImageLibrary.hpp"
#include "ILibraryStore.hpp"

#include <memory>

namespace talay::Library
{
   /*
   *
   * Concrete Image Library Class (Repository)
   *
   * Keeps every library entry cached in memory and writes each mutation
   * through the injected ILibraryStore (Dependency Inversion: the storage
   * engine arrives via the constructor and is never named here). Queries
   * compile the given LibraryFilter into a specification chain and run it
   * over the cache, so live filtering never touches the database. One
   * instance per database — the main window owns it (not a singleton).
   *
   */
   class OImageLibrary final : public IImageLibrary
   {
      public:

         explicit OImageLibrary(std::unique_ptr<Store::ILibraryStore> store);

         OImageLibrary(const OImageLibrary& library) = delete;
         OImageLibrary(OImageLibrary&& library) noexcept = delete;

         ~OImageLibrary() noexcept override;

         OImageLibrary& operator=(const OImageLibrary& library) = delete;
         OImageLibrary& operator=(OImageLibrary&& library) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IImageLibrary overrides
         //

         bool open(const std::wstring& databasePath) override;

         bool add(const LibraryEntry& entry) override;
         bool remove(const std::wstring& filePath) override;
         bool setFavorite(const std::wstring& filePath, bool favorite) override;
         bool removePaths(const std::vector<std::wstring>& filePaths) override;
         bool clear(void) override;

         bool contains(const std::wstring& filePath) const override;
         bool isFavorite(const std::wstring& filePath) const override;

         std::size_t entryCount(void) const override;

         std::vector<std::wstring> allPaths(void) const override;

         std::vector<LibraryEntry> query(const LibraryFilter& filter) const override;

         std::vector<std::wstring> availableFormats(void) const override;
         std::vector<std::wstring> availableFolders(void) const override;

         void onChanged(LibraryChangedCallback callback) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_LIBRARY_OIMAGELIBRARY_HPP
