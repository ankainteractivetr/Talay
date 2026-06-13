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

#ifndef TALAY_LIBRARY_TRANSFER_OLIBRARYTRANSFER_HPP
#define TALAY_LIBRARY_TRANSFER_OLIBRARYTRANSFER_HPP

#include "ILibraryTransfer.hpp"
#include "IImageLibrary.hpp"
#include "framework/Core/Database/IDatabase.hpp"

#include <functional>
#include <memory>

namespace talay::Library::Transfer
{
   // Mints the scratch database an import replays its dump into. Injected so
   // the transfer never names a concrete SQL engine (Dependency Inversion;
   // Abstract Factory as a function).
   using DatabaseFactory = std::function<std::unique_ptr<anka::Core::Database::IDatabase>(void)>;

   /*
   *
   * Concrete Library Transfer Class
   *
   * Owns the dump format end to end. Export walks the repository and writes
   * one self-contained UTF-8 .sql script: the shared library_entry schema,
   * then every entry as an INSERT inside one transaction. Import replays such
   * a script into a scratch in-memory database (arbitrary text never touches
   * the live database), reads the rows back, and merges them into the
   * library — skipping rows already present and rows whose image file no
   * longer exists on this machine, tallying everything into the returned
   * TransferSummary. One instance per library — the main window owns it.
   *
   */
   class OLibraryTransfer final : public ILibraryTransfer
   {
      public:

         OLibraryTransfer(IImageLibrary& library, DatabaseFactory makeDatabase);

         OLibraryTransfer(const OLibraryTransfer& transfer) = delete;
         OLibraryTransfer(OLibraryTransfer&& transfer) noexcept = delete;

         ~OLibraryTransfer() noexcept override;

         OLibraryTransfer& operator=(const OLibraryTransfer& transfer) = delete;
         OLibraryTransfer& operator=(OLibraryTransfer&& transfer) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // ILibraryTransfer overrides
         //

         bool exportTo(const std::wstring& dumpFilePath) override;

         TransferSummary importFrom(const std::wstring& dumpFilePath) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_LIBRARY_TRANSFER_OLIBRARYTRANSFER_HPP
