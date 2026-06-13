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

#ifndef TALAY_LIBRARY_FILTER_IENTRYSPECIFICATION_HPP
#define TALAY_LIBRARY_FILTER_IENTRYSPECIFICATION_HPP

#include "LibraryEntry.hpp"

namespace talay::Library::Filter
{
   /*
   * Abstract Entry Specification Interface (Specification pattern)
   *
   * One yes/no rule over a library entry. Each filter criterion becomes its
   * own small specification, and a composite chains them, so adding a new
   * filter means adding a class — never editing an if-cascade (Open/Closed).
   * Specifications are throwaway policy values built per query; they carry no
   * identity, so they deliberately stay outside the IObject hierarchy.
   *
   */
   class IEntrySpecification
   {
      public:

         virtual ~IEntrySpecification() noexcept = default;

         virtual bool isSatisfiedBy(const LibraryEntry& entry) const = 0;
   };
}

#endif // TALAY_LIBRARY_FILTER_IENTRYSPECIFICATION_HPP
