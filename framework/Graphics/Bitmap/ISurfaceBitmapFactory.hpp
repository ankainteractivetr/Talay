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

#ifndef ANKA_GRAPHICS_BITMAP_ISURFACEBITMAPFACTORY_HPP
#define ANKA_GRAPHICS_BITMAP_ISURFACEBITMAPFACTORY_HPP

#include "ISurfaceBitmap.hpp"
#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Image/ImageSequence.hpp"
#include "framework/Graphics/Surface.hpp"

#include <memory>

namespace anka::Graphics::Bitmap
{
	// Builds the right ISurfaceBitmap for a decoded image (Factory Method): a still
	// for a one-frame sequence, a continuous (animated) bitmap otherwise. It uploads
	// each frame's pixels into the surface via the lower-level bitmap factory, so the
	// caller hands over a decode result and gets back something it can draw and tick
	// — without ever choosing or naming the concrete type.
	//
	// SRP: the lower-level factory mints raw Direct2D bitmaps; this one *assembles* a
	// playable bitmap from a sequence (it composes that factory, it does not extend
	// it). New presentable kinds are added here without touching callers (OCP).
	class ISurfaceBitmapFactory : public anka::Core::Object::IObject
	{
		public:

			virtual ~ISurfaceBitmapFactory() noexcept override = default;

			// Null when the sequence is empty or a frame fails to upload.
			virtual std::unique_ptr<ISurfaceBitmap> create(SurfaceId surface, const Image::ImageSequence& sequence) = 0;
	};
}

#endif // ANKA_GRAPHICS_BITMAP_ISURFACEBITMAPFACTORY_HPP
