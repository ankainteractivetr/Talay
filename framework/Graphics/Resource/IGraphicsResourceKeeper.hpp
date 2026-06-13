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

#ifndef ANKA_GRAPHICS_RESOURCE_IGRAPHICSRESOURCEKEEPER_HPP
#define ANKA_GRAPHICS_RESOURCE_IGRAPHICSRESOURCEKEEPER_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Surface.hpp"

#include <d2d1.h>

// Forward-declared so the interface need not pull in <dwrite.h> / <wincodec.h>.
struct IDWriteFactory;
struct IWICBitmap;

namespace anka::Graphics::Resource
{
	// Owns the durable Direct2D graphics state and nothing else: the shared
	// device-independent factories (D2D, DWrite, WIC) and every surface's
	// offscreen WIC bitmap render target. Surfaces are created on demand — one per
	// viewport or thumbnail — and destroyed when their owner is gone.
	//
	// The renderer and the bitmap factory *borrow* these resources through the
	// accessors below; they never own or release them. This keeps resource
	// lifetime in one place (Single Responsibility) and lets every collaborator
	// depend on this abstraction rather than on raw Direct2D plumbing.
	class IGraphicsResourceKeeper : public anka::Core::Object::IObject
	{
		public:

			virtual ~IGraphicsResourceKeeper() noexcept override = default;

			virtual bool      initialize(void) = 0;
			virtual bool      deinitialize(void) = 0;

			virtual SurfaceId createSurface(const D2D1_SIZE_U& size) = 0;
			virtual bool      resizeSurface(SurfaceId surface, const D2D1_SIZE_U& size) = 0;
			virtual bool      destroySurface(SurfaceId surface) = 0;

			virtual ID2D1RenderTarget* renderTarget(SurfaceId surface) const = 0;
			virtual IWICBitmap*        wicBitmap(SurfaceId surface) const = 0;
			virtual D2D1_SIZE_U        surfaceSize(SurfaceId surface) const = 0;

			virtual IDWriteFactory*    dwriteFactory(void) const = 0;
	};
}

#endif // ANKA_GRAPHICS_RESOURCE_IGRAPHICSRESOURCEKEEPER_HPP
