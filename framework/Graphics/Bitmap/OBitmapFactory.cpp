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


#include "pch.h"
#include "OBitmapFactory.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Resource/OGraphicsResourceKeeper.hpp"

#include <d2d1.h>

#include <cstdint>

using namespace anka::Graphics;
using namespace anka::Graphics::Bitmap;
using namespace anka::Graphics::Resource;
using namespace anka::Core::Object;


// -----------------------------------------------------------------------------
// OBitmapFactory::Impl Definition
// -----------------------------------------------------------------------------
struct OBitmapFactory::Impl
{
	ObjectIdentity m_identity {L"BitmapFactoryObject"};

	//
	// IObject delegation
	//

	const std::wstring& getObjectName(void) const
	{
		return m_identity.name();
	}

	void setObjectName(const std::wstring& name)
	{
		m_identity.setName(name);
	}

	std::uint64_t getObjectId(void) const
	{
		return m_identity.id();
	}

	//
	// IBitmapFactory implementation
	//

	// A D2D bitmap is owned by the render target that mints it, so we resolve the
	// surface's target from the keeper and create the bitmap against it.
	ID2D1RenderTarget* targetFor(SurfaceId surface) const
	{
		return OGraphicsResourceKeeper::getInstance().renderTarget(surface);
	}

	bool createFromWicSource(SurfaceId surface, IWICBitmapSource* source, ID2D1Bitmap** bitmap)
	{
		ID2D1RenderTarget* target = (source && bitmap) ? targetFor(surface) : nullptr;
		if (!target) {
			return false;
		}
		return SUCCEEDED(target->CreateBitmapFromWicBitmap(source, bitmap));
	}

	// Decoder-agnostic entry: raw PBGRA pixels (top-down) from any source
	// (e.g. a FreeImage FIBITMAP the caller converted/flipped to premultiplied BGRA).
	bool createFromMemory(SurfaceId surface, const void* pixels, std::uint32_t stride, const D2D1_SIZE_U& size, ID2D1Bitmap** bitmap)
	{
		ID2D1RenderTarget* target = (pixels && bitmap) ? targetFor(surface) : nullptr;
		if (!target) {
			return false;
		}
		const D2D1_BITMAP_PROPERTIES properties = D2D1::BitmapProperties(
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));
		return SUCCEEDED(target->CreateBitmap(size, pixels, stride, properties, bitmap));
	}
};

// -----------------------------------------------------------------------------
// OBitmapFactory Implementation
// -----------------------------------------------------------------------------
OBitmapFactory::OBitmapFactory() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OBitmapFactory::~OBitmapFactory() noexcept = default;
// -----------------------------------------------------------------------------
OBitmapFactory& OBitmapFactory::getInstance()
{
	static OBitmapFactory s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OBitmapFactory::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OBitmapFactory::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OBitmapFactory::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OBitmapFactory::createFromWicSource(SurfaceId surface, IWICBitmapSource* source, ID2D1Bitmap** bitmap)
{
	return m_pImpl->createFromWicSource(surface, source, bitmap);
}
// -----------------------------------------------------------------------------
bool OBitmapFactory::createFromMemory(SurfaceId surface, const void* pixels, std::uint32_t stride, const D2D1_SIZE_U& size, ID2D1Bitmap** bitmap)
{
	return m_pImpl->createFromMemory(surface, pixels, stride, size, bitmap);
}
// -----------------------------------------------------------------------------
