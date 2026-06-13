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

#ifndef ANKA_GUI_VIEWPORT_OFITORNATIVEFRAMING_HPP
#define ANKA_GUI_VIEWPORT_OFITORNATIVEFRAMING_HPP

#include "IFramingPolicy.hpp"

namespace anka::GUI::Viewport
{
   /*
   *
   * Concrete Fit-or-Native Framing Class
   *
   * The classic image-viewer opening rule: an image that fits the viewport
   * opens at its native size (100%), one that overflows is scaled down just
   * enough to fit — and never scaled up. Stateless; the viewport creates one
   * as its default policy when none is injected.
   *
   */
   class OFitOrNativeFraming final : public IFramingPolicy
   {
      public:

         double zoomPercentFor(const Extent& content, const Extent& viewport) const override;
   };
}

#endif // ANKA_GUI_VIEWPORT_OFITORNATIVEFRAMING_HPP
