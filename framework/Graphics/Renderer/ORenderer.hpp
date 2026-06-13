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

#ifndef ANKA_GRAPHICS_RENDERER_ORENDERER_HPP
#define ANKA_GRAPHICS_RENDERER_ORENDERER_HPP

#include "IRenderer.hpp"

#include <cstdint>
#include <memory>

namespace anka::Graphics::Renderer
{
   class ORenderer final : public IRenderer
   {
		public:

			// Singleton
			static ORenderer& getInstance();

			ORenderer(const ORenderer& picker) = delete;
			ORenderer(ORenderer&& picker) noexcept = delete;

			~ORenderer() noexcept override;

			ORenderer& operator=(const ORenderer& renderer) = delete;
			ORenderer& operator=(ORenderer&& renderer) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// IRenderer overrides
			//

			bool initialize(void) override;
			bool deinitialize(void) override;
			bool beginDraw(SurfaceId surface) override;
			bool clear(const D2D1_COLOR_F& color) override;
			bool setTransform(const D2D1_MATRIX_3X2_F& transform) override;
			bool drawBitmap(const D2D1_RECT_F& source,const D2D1_RECT_F& destination,ID2D1Bitmap* bitmap) override;
			bool drawText(const std::wstring& text,const std::int32_t xCoord,const std::int32_t yCoord,const std::int32_t size,const D2D1_COLOR_F& color) override;
			bool endDraw(void) override;
			bool copyPixels(SurfaceId surface,std::uint8_t* destination,std::uint32_t stride,std::uint32_t size) override;

		private:

			//Singleton COTOR
			ORenderer();

			//
			// IRenderer overrides
			//			

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_GRAPHICS_RENDERER_ORENDERER_HPP
