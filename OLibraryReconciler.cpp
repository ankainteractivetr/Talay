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
#include "OLibraryReconciler.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <iterator>
#include <thread>
#include <utility>
#include <vector>

using namespace talay::Library;
using anka::Core::Object::ObjectIdentity;


// The Windows API default behind the injectable FileProbe seam: a backing file
// counts as present only when it resolves and is not a directory — the same
// cheap attribute query the dump import uses, never a file open.
namespace
{
   bool fileExists(const std::wstring& filePath)
   {
      const DWORD attributes {GetFileAttributesW(filePath.c_str())};

      return attributes != INVALID_FILE_ATTRIBUTES
          && (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
   }
}


// -----------------------------------------------------------------------------
// OLibraryReconciler::Impl Definition
// -----------------------------------------------------------------------------
struct OLibraryReconciler::Impl
{
   ObjectIdentity m_identity {L"LibraryReconcilerObject"};

   IImageLibrary& m_library;
   UiDispatch     m_toUiThread;
   FileProbe      m_probe;

   // Auto-joins on destruction, so a half-finished sweep never outlives us.
   std::jthread m_worker;

   Impl(IImageLibrary& library, UiDispatch toUiThread, FileProbe probe) :
      m_library    {library},
      m_toUiThread {std::move(toUiThread)},
      m_probe      {probe ? std::move(probe) : fileExists}
   {}

   // The snapshot is taken here, on the UI thread, so the worker never reads
   // the library while another mutation is writing it.
   void pruneMissing(void)
   {
      std::vector<std::wstring> paths {m_library.allPaths()};

      m_worker = std::jthread([this, paths = std::move(paths)] { sweep(paths); });
   }

   // Worker body: the disk I/O is the only thing off the UI thread; the removal
   // it discovers is marshalled back so the library mutates where views expect.
   void sweep(const std::vector<std::wstring>& paths)
   {
      std::vector<std::wstring> missing {missingAmong(paths)};
      if (missing.empty() || !m_toUiThread)
         return;

      m_toUiThread([this, missing = std::move(missing)] { m_library.removePaths(missing); });
   }

   std::vector<std::wstring> missingAmong(const std::vector<std::wstring>& paths) const
   {
      std::vector<std::wstring> missing;

      std::copy_if(paths.begin(), paths.end(), std::back_inserter(missing),
                   [this](const std::wstring& path) { return !m_probe(path); });
      return missing;
   }
};

// -----------------------------------------------------------------------------
// OLibraryReconciler Implementation
// -----------------------------------------------------------------------------
OLibraryReconciler::OLibraryReconciler(IImageLibrary& library,
                                       UiDispatch toUiThread,
                                       FileProbe probe) :
   m_pImpl {std::make_unique<Impl>(library, std::move(toUiThread), std::move(probe))}
{}
// -----------------------------------------------------------------------------
OLibraryReconciler::~OLibraryReconciler() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OLibraryReconciler::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OLibraryReconciler::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OLibraryReconciler::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OLibraryReconciler::pruneMissing(void)
{
   m_pImpl->pruneMissing();
}
// -----------------------------------------------------------------------------
