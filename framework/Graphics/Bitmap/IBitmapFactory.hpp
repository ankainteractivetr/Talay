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

#ifndef ANKA_GRAPHICS_BITMAP_IBITMAPFACTORY_HPP
#define ANKA_GRAPHICS_BITMAP_IBITMAPFACTORY_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Surface.hpp"

#include <d2d1.h>

#include <cstdint>

// Forward-declared so the interface need not pull in <wincodec.h>.
struct IWICBitmapSource;

namespace anka::Graphics::Bitmap
{
	// Creates Direct2D bitmaps for a surface — the single responsibility that used
	// to live on the renderer. A D2D bitmap is bound to the render target that
	// created it, so each bitmap belongs to exactly one surface; the factory
	// borrows that surface's target from the graphics resource keeper.
	//
	// Decoding lives elsewhere (the planned ImageFileOpener): WIC yields an
	// IWICBitmapSource for createFromWicSource, while a fallback decoder such as
	// FreeImage yields raw premultiplied-BGRA, top-down pixels for
	// createFromMemory. The factory stays decoder-agnostic — it only uploads.
	class IBitmapFactory : public anka::Core::Object::IObject
	{
		public:

			virtual ~IBitmapFactory() noexcept override = default;

			virtual bool createFromWicSource(SurfaceId surface, IWICBitmapSource* source, ID2D1Bitmap** bitmap) = 0;
			virtual bool createFromMemory(SurfaceId surface, const void* pixels, std::uint32_t stride, const D2D1_SIZE_U& size, ID2D1Bitmap** bitmap) = 0;
	};
}

#endif // ANKA_GRAPHICS_BITMAP_IBITMAPFACTORY_HPP
