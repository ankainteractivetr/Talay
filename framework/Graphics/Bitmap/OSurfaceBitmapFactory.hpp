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

#ifndef ANKA_GRAPHICS_BITMAP_OSURFACEBITMAPFACTORY_HPP
#define ANKA_GRAPHICS_BITMAP_OSURFACEBITMAPFACTORY_HPP

#include "ISurfaceBitmapFactory.hpp"

#include <memory>

namespace anka::Graphics::Bitmap
{
	/*
	*
	* Concrete Surface Bitmap Factory Class
	*
	*/
	class OSurfaceBitmapFactory final : public ISurfaceBitmapFactory
	{
		public:

			// Singleton
			static OSurfaceBitmapFactory& getInstance();

			OSurfaceBitmapFactory(const OSurfaceBitmapFactory& factory) = delete;
			OSurfaceBitmapFactory(OSurfaceBitmapFactory&& factory) noexcept = delete;

			~OSurfaceBitmapFactory() noexcept override;

			OSurfaceBitmapFactory& operator=(const OSurfaceBitmapFactory& factory) = delete;
			OSurfaceBitmapFactory& operator=(OSurfaceBitmapFactory&& factory) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// ISurfaceBitmapFactory overrides
			//

			std::unique_ptr<ISurfaceBitmap> create(SurfaceId surface, const Image::ImageSequence& sequence) override;

		private:

			// Singleton COTOR
			OSurfaceBitmapFactory();

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_GRAPHICS_BITMAP_OSURFACEBITMAPFACTORY_HPP
