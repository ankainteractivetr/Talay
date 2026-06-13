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

#ifndef ANKA_GRAPHICS_RESOURCE_OGRAPHICSRESOURCEKEEPER_HPP
#define ANKA_GRAPHICS_RESOURCE_OGRAPHICSRESOURCEKEEPER_HPP

#include "IGraphicsResourceKeeper.hpp"

#include <cstdint>
#include <memory>

namespace anka::Graphics::Resource
{
	/*
	*
	* Concrete Graphics Resource Keeper Class
	*
	*/
	class OGraphicsResourceKeeper final : public IGraphicsResourceKeeper
	{
		public:

			// Singleton
			static OGraphicsResourceKeeper& getInstance();

			OGraphicsResourceKeeper(const OGraphicsResourceKeeper& keeper) = delete;
			OGraphicsResourceKeeper(OGraphicsResourceKeeper&& keeper) noexcept = delete;

			~OGraphicsResourceKeeper() noexcept override;

			OGraphicsResourceKeeper& operator=(const OGraphicsResourceKeeper& keeper) = delete;
			OGraphicsResourceKeeper& operator=(OGraphicsResourceKeeper&& keeper) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// IGraphicsResourceKeeper overrides
			//

			bool      initialize(void) override;
			bool      deinitialize(void) override;

			SurfaceId createSurface(const D2D1_SIZE_U& size) override;
			bool      resizeSurface(SurfaceId surface, const D2D1_SIZE_U& size) override;
			bool      destroySurface(SurfaceId surface) override;

			ID2D1RenderTarget* renderTarget(SurfaceId surface) const override;
			IWICBitmap*        wicBitmap(SurfaceId surface) const override;
			D2D1_SIZE_U        surfaceSize(SurfaceId surface) const override;

			IDWriteFactory*    dwriteFactory(void) const override;

		private:

			// Singleton COTOR
			OGraphicsResourceKeeper();

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_GRAPHICS_RESOURCE_OGRAPHICSRESOURCEKEEPER_HPP
