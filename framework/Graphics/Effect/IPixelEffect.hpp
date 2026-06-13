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

#ifndef ANKA_GRAPHICS_EFFECT_IPIXELEFFECT_HPP
#define ANKA_GRAPHICS_EFFECT_IPIXELEFFECT_HPP

#include "framework/Core/Object/IObject.hpp"
#include "PixelView.hpp"

namespace anka::Graphics::Effect
{
   /*
   * Abstract Pixel Effect Interface (Strategy)
   *
   * One read-only view effect: a transform over a frame of premultiplied BGRA
   * pixels, applied to whatever copy the caller chooses to sacrifice — never
   * to a source image. Implementations are immutable after construction and
   * carry no per-frame state, so a single instance serves any number of
   * frames and callers may share it freely.
   *
   */
   class IPixelEffect : public anka::Core::Object::IObject
   {
      public:

         virtual ~IPixelEffect() noexcept override = default;

         // Transform 'frame' in place. The frame keeps its premultiplied BGRA
         // layout, so the result can go wherever the original could.
         virtual void apply(const PixelView& frame) const = 0;
   };
}

#endif // ANKA_GRAPHICS_EFFECT_IPIXELEFFECT_HPP
