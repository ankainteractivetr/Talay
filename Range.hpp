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

#ifndef TALAY_LIBRARY_RANGE_HPP
#define TALAY_LIBRARY_RANGE_HPP

#include <optional>

namespace talay::Library
{
   // An optionally bounded, inclusive [minimum, maximum] interval — the shape
   // every "between" filter (date, size, dimensions) shares. An unset bound
   // does not constrain, so a default Range contains every value; that makes
   // "filter inactive" and "bounds unset" the same state, with no extra flag.
   template <typename Value>
   class Range
   {
      public:

         std::optional<Value> minimum;
         std::optional<Value> maximum;

         bool unbounded(void) const
         {
            return !minimum.has_value() && !maximum.has_value();
         }

         bool contains(const Value& value) const
         {
            return (!minimum.has_value() || *minimum <= value)
                && (!maximum.has_value() || value <= *maximum);
         }
   };
}

#endif // TALAY_LIBRARY_RANGE_HPP
