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
#include "OSpecificationFactory.hpp"
#include "OEntrySpecifications.hpp"

#include <vector>

using namespace talay::Library;
using namespace talay::Library::Filter;


namespace
{
   using SpecificationList = std::vector<std::unique_ptr<IEntrySpecification>>;

   void appendTextRules(const LibraryFilter& filter, SpecificationList& rules)
   {
      if (!filter.nameContains.empty())
         rules.push_back(std::make_unique<ONameContainsSpecification>(filter.nameContains));

      if (!filter.formats.empty())
         rules.push_back(std::make_unique<OFormatSetSpecification>(filter.formats));

      if (!filter.folder.empty())
         rules.push_back(std::make_unique<OFolderSpecification>(filter.folder));
   }

   void appendRangeRules(const LibraryFilter& filter, SpecificationList& rules)
   {
      if (!filter.modifiedUtc.unbounded())
         rules.push_back(std::make_unique<ODateRangeSpecification>(filter.modifiedUtc));

      if (!filter.sizeKb.unbounded())
         rules.push_back(std::make_unique<OSizeRangeSpecification>(filter.sizeKb));

      if (!filter.width.unbounded() || !filter.height.unbounded())
         rules.push_back(std::make_unique<ODimensionRangeSpecification>(filter.width, filter.height));
   }

   void appendFlagRules(const LibraryFilter& filter, SpecificationList& rules)
   {
      if (filter.favoritesOnly)
         rules.push_back(std::make_unique<OFavoriteSpecification>());

      if (!filter.includeStills || !filter.includeAnimated)
         rules.push_back(std::make_unique<OMotionSpecification>(filter.includeStills, filter.includeAnimated));
   }
}


// -----------------------------------------------------------------------------
// OSpecificationFactory Implementation
// -----------------------------------------------------------------------------
std::unique_ptr<IEntrySpecification> OSpecificationFactory::fromFilter(const LibraryFilter& filter)
{
   SpecificationList rules;

   appendTextRules(filter, rules);
   appendRangeRules(filter, rules);
   appendFlagRules(filter, rules);

   return std::make_unique<OAndSpecification>(std::move(rules));
}
// -----------------------------------------------------------------------------
