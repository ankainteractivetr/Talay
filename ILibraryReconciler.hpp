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

#ifndef TALAY_LIBRARY_ILIBRARYRECONCILER_HPP
#define TALAY_LIBRARY_ILIBRARYRECONCILER_HPP

#include "framework/Core/Object/IObject.hpp"

namespace talay::Library
{
   /*
   * Abstract Library Reconciler Interface
   *
   * Keeps the library honest about what is still on disk: a backing file that
   * was deleted or renamed since it was added must stop being listed. The
   * checking is pure disk I/O and potentially over thousands of entries, so an
   * implementation is free to do it off the UI thread — this seam only promises
   * the work happens, never on which thread the files are touched.
   *
   */
   class ILibraryReconciler : public anka::Core::Object::IObject
   {
      public:

         virtual ~ILibraryReconciler() noexcept override = default;

         // Verify every entry's backing file and drop the ones whose file is
         // gone. Returns at once; the library is mutated when the sweep lands.
         virtual void pruneMissing(void) = 0;
   };
}

#endif // TALAY_LIBRARY_ILIBRARYRECONCILER_HPP
