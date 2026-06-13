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

#ifndef TALAY_LIBRARY_FILTER_OENTRYSPECIFICATIONS_HPP
#define TALAY_LIBRARY_FILTER_OENTRYSPECIFICATIONS_HPP

#include "IEntrySpecification.hpp"
#include "Range.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace talay::Library::Filter
{
   // The concrete rules — one tiny class per filter criterion, each holding
   // only the values it compares against. They live together because they are
   // a closed family: the factory is their only client.

   // Case-insensitive "file name contains" match.
   class ONameContainsSpecification final : public IEntrySpecification
   {
      public:

         explicit ONameContainsSpecification(const std::wstring& needle);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         std::wstring m_needleUpper;
   };

   // Entry's format is one of the chosen set.
   class OFormatSetSpecification final : public IEntrySpecification
   {
      public:

         explicit OFormatSetSpecification(std::vector<std::wstring> formats);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         std::vector<std::wstring> m_formats;
   };

   // File's last-write stamp falls inside the chosen period.
   class ODateRangeSpecification final : public IEntrySpecification
   {
      public:

         explicit ODateRangeSpecification(const Range<std::int64_t>& period);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         Range<std::int64_t> m_period;
   };

   // On-disk size in whole kilobytes falls inside the chosen bounds.
   class OSizeRangeSpecification final : public IEntrySpecification
   {
      public:

         explicit OSizeRangeSpecification(const Range<std::uint64_t>& kilobytes);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         Range<std::uint64_t> m_kilobytes;
   };

   // Pixel width and height each fall inside their chosen bounds.
   class ODimensionRangeSpecification final : public IEntrySpecification
   {
      public:

         ODimensionRangeSpecification(const Range<std::uint32_t>& width,
                                      const Range<std::uint32_t>& height);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         Range<std::uint32_t> m_width;
         Range<std::uint32_t> m_height;
   };

   // Entry carries the user's favorite mark.
   class OFavoriteSpecification final : public IEntrySpecification
   {
      public:

         bool isSatisfiedBy(const LibraryEntry& entry) const override;
   };

   // Entry's motion kind (still / animated) is one the user kept enabled.
   class OMotionSpecification final : public IEntrySpecification
   {
      public:

         OMotionSpecification(bool includeStills, bool includeAnimated);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         bool m_includeStills;
         bool m_includeAnimated;
   };

   // Entry lives in the chosen source folder.
   class OFolderSpecification final : public IEntrySpecification
   {
      public:

         explicit OFolderSpecification(std::wstring folder);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         std::wstring m_folder;
   };

   // Composite: satisfied only when every child is (an empty chain matches
   // everything — the neutral element, so "no active filters" needs no case).
   class OAndSpecification final : public IEntrySpecification
   {
      public:

         explicit OAndSpecification(std::vector<std::unique_ptr<IEntrySpecification>> children);

         bool isSatisfiedBy(const LibraryEntry& entry) const override;

      private:

         std::vector<std::unique_ptr<IEntrySpecification>> m_children;
   };
}

#endif // TALAY_LIBRARY_FILTER_OENTRYSPECIFICATIONS_HPP
