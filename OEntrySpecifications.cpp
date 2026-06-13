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
#include "OEntrySpecifications.hpp"

#include <algorithm>
#include <cwctype>
#include <utility>

using namespace talay::Library;
using namespace talay::Library::Filter;


namespace
{
   std::wstring toUpper(std::wstring text)
   {
      std::transform(text.begin(), text.end(), text.begin(),
                     [](wchar_t character) { return static_cast<wchar_t>(std::towupper(character)); });
      return text;
   }
}


// -----------------------------------------------------------------------------
// ONameContainsSpecification
// -----------------------------------------------------------------------------
ONameContainsSpecification::ONameContainsSpecification(const std::wstring& needle) :
   m_needleUpper {toUpper(needle)}
{}
// -----------------------------------------------------------------------------
bool ONameContainsSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return toUpper(entry.fileName).find(m_needleUpper) != std::wstring::npos;
}

// -----------------------------------------------------------------------------
// OFormatSetSpecification
// -----------------------------------------------------------------------------
OFormatSetSpecification::OFormatSetSpecification(std::vector<std::wstring> formats) :
   m_formats {std::move(formats)}
{}
// -----------------------------------------------------------------------------
bool OFormatSetSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return std::find(m_formats.begin(), m_formats.end(), entry.format) != m_formats.end();
}

// -----------------------------------------------------------------------------
// ODateRangeSpecification
// -----------------------------------------------------------------------------
ODateRangeSpecification::ODateRangeSpecification(const Range<std::int64_t>& period) :
   m_period {period}
{}
// -----------------------------------------------------------------------------
bool ODateRangeSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return m_period.contains(entry.modifiedUtc);
}

// -----------------------------------------------------------------------------
// OSizeRangeSpecification
// -----------------------------------------------------------------------------
OSizeRangeSpecification::OSizeRangeSpecification(const Range<std::uint64_t>& kilobytes) :
   m_kilobytes {kilobytes}
{}
// -----------------------------------------------------------------------------
bool OSizeRangeSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return m_kilobytes.contains(entry.fileSizeBytes / 1024u);
}

// -----------------------------------------------------------------------------
// ODimensionRangeSpecification
// -----------------------------------------------------------------------------
ODimensionRangeSpecification::ODimensionRangeSpecification(const Range<std::uint32_t>& width,
                                                           const Range<std::uint32_t>& height) :
   m_width  {width},
   m_height {height}
{}
// -----------------------------------------------------------------------------
bool ODimensionRangeSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return m_width.contains(entry.width) && m_height.contains(entry.height);
}

// -----------------------------------------------------------------------------
// OFavoriteSpecification
// -----------------------------------------------------------------------------
bool OFavoriteSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return entry.isFavorite;
}

// -----------------------------------------------------------------------------
// OMotionSpecification
// -----------------------------------------------------------------------------
OMotionSpecification::OMotionSpecification(bool includeStills, bool includeAnimated) :
   m_includeStills   {includeStills},
   m_includeAnimated {includeAnimated}
{}
// -----------------------------------------------------------------------------
bool OMotionSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return entry.animated() ? m_includeAnimated : m_includeStills;
}

// -----------------------------------------------------------------------------
// OFolderSpecification
// -----------------------------------------------------------------------------
OFolderSpecification::OFolderSpecification(std::wstring folder) :
   m_folder {std::move(folder)}
{}
// -----------------------------------------------------------------------------
bool OFolderSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return entry.folder == m_folder;
}

// -----------------------------------------------------------------------------
// OAndSpecification
// -----------------------------------------------------------------------------
OAndSpecification::OAndSpecification(std::vector<std::unique_ptr<IEntrySpecification>> children) :
   m_children {std::move(children)}
{}
// -----------------------------------------------------------------------------
bool OAndSpecification::isSatisfiedBy(const LibraryEntry& entry) const
{
   return std::all_of(m_children.begin(), m_children.end(),
                      [&entry](const auto& child) { return child->isSatisfiedBy(entry); });
}
// -----------------------------------------------------------------------------
