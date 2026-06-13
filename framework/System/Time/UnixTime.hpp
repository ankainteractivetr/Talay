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

#ifndef ANKA_SYSTEM_TIME_UNIXTIME_HPP
#define ANKA_SYSTEM_TIME_UNIXTIME_HPP

// FILETIME is the currency here, so the header has to name it — this is a
// Win32 time utility and makes no pretence otherwise.
#include "framework/System/Windows/windows.hpp"

#include <cstdint>

namespace anka::System::Time
{
   // Whole seconds since the Unix epoch (1970-01-01 UTC) for a given FILETIME.
   // FILETIME counts 100-nanosecond ticks from 1601, so the value is rebased by
   // the 11'644'473'600-second gap between the two epochs and rescaled from
   // ticks to seconds — the stamp the library and recent-files rows carry.
   std::int64_t toUnixSeconds(const FILETIME& fileTime);

   // The wall clock right now on that same Unix-seconds scale, read straight
   // from the system clock.
   std::int64_t currentUnixSeconds(void);
}

#endif // ANKA_SYSTEM_TIME_UNIXTIME_HPP
