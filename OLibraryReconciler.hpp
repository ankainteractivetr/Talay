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

#ifndef TALAY_LIBRARY_OLIBRARYRECONCILER_HPP
#define TALAY_LIBRARY_OLIBRARYRECONCILER_HPP

#include "ILibraryReconciler.hpp"
#include "IImageLibrary.hpp"

#include <functional>
#include <memory>
#include <string>

namespace talay::Library
{
   // The reconciler's two environment touch-points, injectable so tests can
   // fake the file system and run the work inline (Strategy). Left empty at
   // the call site, the Windows API / synchronous defaults step in.
   using FileProbe = std::function<bool(const std::wstring& /*filePath*/)>;

   // Hands a unit of work to the thread the library lives on. The sweep does
   // its disk I/O on a worker thread, then marshals the removal back through
   // this, so every library mutation still happens where the views expect it.
   using UiDispatch = std::function<void(std::function<void(void)> /*work*/)>;

   /*
   *
   * Concrete Library Reconciler Class
   *
   * Snapshots the library's paths on the calling (UI) thread, tests their
   * existence on a worker thread through the injected FileProbe — the cheap
   * GetFileAttributesW, never an open — and posts a single batch removal back
   * through the injected UiDispatch. The library and its store are touched
   * only on the UI thread, so the in-memory cache never races the worker; the
   * disk I/O, the one part that can stall, is the only thing off-thread.
   * Performance by construction: one snapshot, one off-thread pass, one batch
   * delete, one view refresh — and the live filter path is left untouched.
   *
   */
   class OLibraryReconciler final : public ILibraryReconciler
   {
      public:

         OLibraryReconciler(IImageLibrary& library,
                            UiDispatch toUiThread,
                            FileProbe probe = {});

         OLibraryReconciler(const OLibraryReconciler& reconciler) = delete;
         OLibraryReconciler(OLibraryReconciler&& reconciler) noexcept = delete;

         ~OLibraryReconciler() noexcept override;

         OLibraryReconciler& operator=(const OLibraryReconciler& reconciler) = delete;
         OLibraryReconciler& operator=(OLibraryReconciler&& reconciler) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // ILibraryReconciler overrides
         //

         void pruneMissing(void) override;

      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // TALAY_LIBRARY_OLIBRARYRECONCILER_HPP
