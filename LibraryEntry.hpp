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

#ifndef TALAY_LIBRARY_LIBRARYENTRY_HPP
#define TALAY_LIBRARY_LIBRARYENTRY_HPP

#include <cstdint>
#include <string>

namespace talay::Library
{
   // One image the user added to the library, exactly as a database row: file
   // identity, the facts every filter runs on, and the user's favorite mark.
   // Timestamps are raw UTC seconds since the Unix epoch (numbers compare;
   // formatting civil time is a presentation concern, left to the view).
   //
   // Plain value type that owns its data: copyable and movable by default,
   // no manual resource management. id == 0 means "not yet persisted".
   class LibraryEntry
   {
      public:

         std::int64_t  id {0};

         std::wstring  filePath;
         std::wstring  fileName;
         std::wstring  folder;
         std::wstring  format;            // container, e.g. L"PNG"

         std::uint64_t fileSizeBytes {0};

         std::uint32_t width      {0};
         std::uint32_t height     {0};
         std::uint32_t frameCount {1};

         std::int64_t  modifiedUtc {0};   // file's last-write stamp
         std::int64_t  addedUtc    {0};   // when it entered the library

         bool isFavorite {false};

         bool animated(void) const
         {
            return frameCount > 1;
         }
   };
}

#endif // TALAY_LIBRARY_LIBRARYENTRY_HPP
