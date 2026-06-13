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

#ifndef ANKA_GRAPHICS_EFFECT_LOOKUPTABLE_HPP
#define ANKA_GRAPHICS_EFFECT_LOOKUPTABLE_HPP

#include <array>
#include <cstddef>
#include <cstdint>

namespace anka::Graphics::Effect
{
   // A tone curve as data: one output value per possible channel byte, applied
   // identically to R, G and B (alpha is never curved). Where the colour
   // matrix is linear maths, the table holds every non-linear curve — gamma,
   // levels, film response — without the engine ever changing.
   struct LookupTable
   {
      static constexpr std::size_t k_entries {256};

      std::array<std::uint8_t, k_entries> values {};
   };
}

#endif // ANKA_GRAPHICS_EFFECT_LOOKUPTABLE_HPP
