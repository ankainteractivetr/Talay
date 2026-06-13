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

#ifndef ANKA_GRAPHICS_RENDERER_IRENDERER_HPP
#define ANKA_GRAPHICS_RENDERER_IRENDERER_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Surface.hpp"
#include "framework/System/Windows/windows.hpp"

#include <d2d1.h>

#include <cstdint>

namespace anka::Graphics::Renderer
{
   // On-demand Direct2D renderer. It paints one surface at a time: beginDraw
   // selects that surface's target (owned by the graphics resource keeper), the
   // draw calls fill it, endDraw flushes, and copyPixels hands the pixels to a
   // XAML WriteableBitmap. A surface is redrawn only when its input changes.
   //
   // Surface lifetime (create/resize/destroy) belongs to the keeper, and bitmap
   // creation to the bitmap factory — the renderer only draws.
   class IRenderer : public anka::Core::Object::IObject
   {
      public:

         virtual ~IRenderer() noexcept override = default;

         virtual bool initialize(void) = 0;
         virtual bool deinitialize(void) = 0;
         virtual bool beginDraw(SurfaceId surface) = 0;
         virtual bool clear(const D2D1_COLOR_F& color) = 0;
         virtual bool setTransform(const D2D1_MATRIX_3X2_F& transform) = 0;
         virtual bool drawBitmap(const D2D1_RECT_F& source, const D2D1_RECT_F& destination, ID2D1Bitmap* bitmap) = 0;
         virtual bool drawText(const std::wstring& text, const std::int32_t xCoord, const std::int32_t yCoord, const std::int32_t size, const D2D1_COLOR_F& color) = 0;
         virtual bool endDraw(void) = 0;
         virtual bool copyPixels(SurfaceId surface, std::uint8_t* destination, std::uint32_t stride, std::uint32_t size) = 0;
   };
}

#endif // ANKA_GRAPHICS_RENDERER_IRENDERER_HPP