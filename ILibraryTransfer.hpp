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

#ifndef TALAY_LIBRARY_TRANSFER_ILIBRARYTRANSFER_HPP
#define TALAY_LIBRARY_TRANSFER_ILIBRARYTRANSFER_HPP

#include "framework/Core/Object/IObject.hpp"

#include <cstddef>
#include <string>

namespace talay::Library::Transfer
{
   // What an import achieved, for the host's summary dialog: every dump row
   // was either added, skipped as already present, or skipped because its
   // image file no longer exists on this machine.
   class TransferSummary
   {
      public:

         bool succeeded {false};

         std::size_t imported        {0};
         std::size_t skippedExisting {0};
         std::size_t skippedMissing  {0};
   };

   /*
   * Abstract Library Transfer Interface
   *
   * Moves the library through portable SQL dump files: exportTo() writes
   * every entry as one self-contained .sql script (schema + INSERTs), and
   * importFrom() replays such a script and merges its rows into the live
   * library. How a dump is generated and replayed stays an implementation
   * detail behind this interface.
   *
   */
   class ILibraryTransfer : public anka::Core::Object::IObject
   {
      public:

         virtual ~ILibraryTransfer() noexcept override = default;

         virtual bool exportTo(const std::wstring& dumpFilePath) = 0;

         virtual TransferSummary importFrom(const std::wstring& dumpFilePath) = 0;
   };
}

#endif // TALAY_LIBRARY_TRANSFER_ILIBRARYTRANSFER_HPP
