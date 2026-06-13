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

#ifndef ANKA_CORE_PAGING_PAGER_HPP
#define ANKA_CORE_PAGING_PAGER_HPP

#include <algorithm>
#include <cstddef>

namespace anka::Core::Paging
{
   /*
   * Pager Class
   *
   * Pure page arithmetic over a linearly indexed item list: give it the item
   * count and a page size, and it answers which [firstItem, endItem) window
   * the current page covers. It owns nothing and renders nothing — a view
   * layer drives it and reads it (Single Responsibility). A plain value type
   * with copy semantics, so it deliberately stays outside the IObject
   * hierarchy. An empty list still reports one (empty) page, sparing every
   * "page 1 / 1" display a special case.
   *
   */
   class Pager
   {
      public:

         // Changing the size keeps the currently visible first item on screen
         // by re-deriving the page that contains it.
         void setPageSize(std::size_t size)
         {
            const std::size_t anchor {firstItem()};

            m_pageSize  = std::max<std::size_t>(size, 1);
            m_pageIndex = anchor / m_pageSize;
            clampPage();
         }

         // A shrinking list pulls the current page back into range; the page
         // is otherwise left where it is.
         void setItemCount(std::size_t count)
         {
            m_itemCount = count;
            clampPage();
         }

         void first(void)
         {
            m_pageIndex = 0;
         }

         // Step forward / back; false means the edge was already reached and
         // nothing changed, so callers only re-render on true.
         bool next(void)
         {
            if (!hasNext())
               return false;

            ++m_pageIndex;
            return true;
         }

         bool previous(void)
         {
            if (!hasPrevious())
               return false;

            --m_pageIndex;
            return true;
         }

         // Jump to the page that contains 'item'; false means that page was
         // already current, so callers only re-render on true.
         bool goToItem(std::size_t item)
         {
            const std::size_t page {std::min<std::size_t>(item / m_pageSize, pageCount() - 1)};
            if (page == m_pageIndex)
               return false;

            m_pageIndex = page;
            return true;
         }

         std::size_t pageSize(void) const
         {
            return m_pageSize;
         }

         // 1-based, ready for a "page X / Y" display.
         std::size_t pageNumber(void) const
         {
            return m_pageIndex + 1;
         }

         std::size_t pageCount(void) const
         {
            return std::max<std::size_t>((m_itemCount + m_pageSize - 1) / m_pageSize, 1);
         }

         // Explicit template arguments keep windows.h's min/max macros from
         // eating these calls (the bare names would be macro-expanded).
         std::size_t firstItem(void) const
         {
            return std::min<std::size_t>(m_pageIndex * m_pageSize, m_itemCount);
         }

         // One past the last item of the page, never beyond the list.
         std::size_t endItem(void) const
         {
            return std::min<std::size_t>(firstItem() + m_pageSize, m_itemCount);
         }

         std::size_t itemsOnPage(void) const
         {
            return endItem() - firstItem();
         }

         bool hasPrevious(void) const
         {
            return m_pageIndex > 0;
         }

         bool hasNext(void) const
         {
            return m_pageIndex + 1 < pageCount();
         }

      private:

         void clampPage(void)
         {
            m_pageIndex = std::min<std::size_t>(m_pageIndex, pageCount() - 1);
         }

         std::size_t m_pageSize  {50};
         std::size_t m_itemCount {0};
         std::size_t m_pageIndex {0};   // 0-based internally
   };
}

#endif // ANKA_CORE_PAGING_PAGER_HPP
