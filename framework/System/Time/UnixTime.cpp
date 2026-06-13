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
#include "UnixTime.hpp"

// -----------------------------------------------------------------------------
std::int64_t anka::System::Time::toUnixSeconds(const FILETIME& fileTime)
{
   ULARGE_INTEGER ticks {};
   ticks.HighPart = fileTime.dwHighDateTime;
   ticks.LowPart  = fileTime.dwLowDateTime;

   return static_cast<std::int64_t>(ticks.QuadPart / 10'000'000ULL) - 11'644'473'600LL;
}
// -----------------------------------------------------------------------------
std::int64_t anka::System::Time::currentUnixSeconds(void)
{
   // "Now" arrives as a FILETIME, so the same rebase serves both entry points
   // and the magic constant lives in exactly one place.
   FILETIME now {};
   GetSystemTimeAsFileTime(&now);

   return toUnixSeconds(now);
}
// -----------------------------------------------------------------------------
