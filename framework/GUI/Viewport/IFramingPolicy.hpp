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

#ifndef ANKA_GUI_VIEWPORT_IFRAMINGPOLICY_HPP
#define ANKA_GUI_VIEWPORT_IFRAMINGPOLICY_HPP

#include "Extent.hpp"

namespace anka::GUI::Viewport
{
   /*
   * Abstract Framing Policy Interface (Strategy)
   *
   * Decides the zoom percent a freshly framed image opens at, given the
   * image's native extent and the viewport's extent in the same coordinate
   * space. Kept a stateless, identity-less strategy (ISP) so the opening rule
   * can be swapped — always-fit, always-native, fill — without touching the
   * viewport (OCP); the viewport depends only on this abstraction (DIP).
   *
   */
   class IFramingPolicy
   {
      public:

         virtual ~IFramingPolicy() noexcept = default;

         // The zoom percent (100 == native size) 'content' should open at
         // inside 'viewport'; both extents in the same coordinate space.
         virtual double zoomPercentFor(const Extent& content, const Extent& viewport) const = 0;
   };
}

#endif // ANKA_GUI_VIEWPORT_IFRAMINGPOLICY_HPP
