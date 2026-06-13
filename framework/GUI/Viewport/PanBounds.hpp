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

#ifndef ANKA_GUI_VIEWPORT_PANBOUNDS_HPP
#define ANKA_GUI_VIEWPORT_PANBOUNDS_HPP

#include "Extent.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace anka::GUI::Viewport
{
   // The symmetric pan limits of a centred image: how far its centre may drift
   // from the viewport's centre, per axis, before an image edge would slide
   // into view. A zero limit (image no larger than the viewport on that axis)
   // pins the image centred. Pure value — WinUI-free, so the maths is testable.
   class PanBounds
   {
      public:

         PanBounds(double maxX, double maxY) :
            m_maxX {maxX},
            m_maxY {maxY}
         {}

         double clampX(double offset) const
         {
            return std::clamp(offset, -m_maxX, m_maxX);
         }

         double clampY(double offset) const
         {
            return std::clamp(offset, -m_maxY, m_maxY);
         }

         // The image overflows the viewport on at least one axis, so a drag
         // would actually move it — what tells a cursor it can pan here.
         bool allowsPan(void) const
         {
            return m_maxX > 0.0 || m_maxY > 0.0;
         }

      private:

         double m_maxX;
         double m_maxY;
   };

   // Bounds for 'content' scaled by 'scale', rotated by 'rotationDegrees' and
   // shown centred inside 'viewport': the rotated image's axis-aligned box may
   // pan by half its overhang per axis, and not at all where it does not
   // overflow. 'scale' must map the content into the viewport's coordinates.
   inline PanBounds makePanBounds(const Extent& content, double scale,
                                  double rotationDegrees, const Extent& viewport)
   {
      const double radians = rotationDegrees * std::numbers::pi / 180.0;
      const double cosine  = std::abs(std::cos(radians));
      const double sine    = std::abs(std::sin(radians));

      const double shownWidth  = (content.width * cosine + content.height * sine) * scale;
      const double shownHeight = (content.width * sine + content.height * cosine) * scale;

      return PanBounds {(std::max)(0.0, (shownWidth - viewport.width) / 2.0),
                        (std::max)(0.0, (shownHeight - viewport.height) / 2.0)};
   }
}

#endif // ANKA_GUI_VIEWPORT_PANBOUNDS_HPP
