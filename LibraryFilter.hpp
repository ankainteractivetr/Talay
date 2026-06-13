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

#ifndef TALAY_LIBRARY_LIBRARYFILTER_HPP
#define TALAY_LIBRARY_LIBRARYFILTER_HPP

#include "Range.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace talay::Library
{
   // Everything the library panel can narrow the entry list by, gathered into
   // one value the view fills and the query layer consumes — the single
   // currency between GUI and filtering, so neither knows the other's types.
   //
   // The default-constructed filter matches every entry: empty text means
   // "any", an empty format set means "all formats", an unbounded Range does
   // not constrain, and both motion kinds are included.
   class LibraryFilter
   {
      public:

         std::wstring nameContains;            // case-insensitive substring

         std::vector<std::wstring> formats;    // e.g. {L"PNG", L"GIF"}

         Range<std::int64_t>  modifiedUtc;     // seconds since the Unix epoch
         Range<std::uint64_t> sizeKb;
         Range<std::uint32_t> width;
         Range<std::uint32_t> height;

         bool favoritesOnly   {false};
         bool includeStills   {true};
         bool includeAnimated {true};

         std::wstring folder;                  // exact source folder
   };
}

#endif // TALAY_LIBRARY_LIBRARYFILTER_HPP
