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
#include "OGraphicsResourceKeeper.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <cstdint>
#include <unordered_map>
#include <utility>

// d2d1.lib / dwrite.lib / windowscodecs.lib are linked via the consuming
// project's Linker > AdditionalDependencies, not #pragma comment(lib).

using namespace anka::Graphics;
using namespace anka::Graphics::Resource;
using namespace anka::Core::Object;


// -----------------------------------------------------------------------------
// OGraphicsResourceKeeper::Impl Definition
// -----------------------------------------------------------------------------
struct OGraphicsResourceKeeper::Impl
{
	// One offscreen, software (no D3D device) render target per surface: a WIC
	// bitmap in PBGRA wrapped by a D2D WIC bitmap render target. The renderer
	// draws into renderTarget; copyPixels reads back from wicBitmap.
	struct SurfaceTarget
	{
		winrt::com_ptr<IWICBitmap>        wicBitmap;
		winrt::com_ptr<ID2D1RenderTarget> renderTarget;
		D2D1_SIZE_U                       size;
	};

	ObjectIdentity                               m_identity {L"GraphicsResourceKeeperObject"};

	winrt::com_ptr<ID2D1Factory>                 m_d2dFactory;
	winrt::com_ptr<IDWriteFactory>               m_dwriteFactory;
	winrt::com_ptr<IWICImagingFactory>           m_wicFactory;

	std::unordered_map<SurfaceId, SurfaceTarget> m_surfaces;
	SurfaceId                                    m_nextId {k_invalidSurface + 1};

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
	// Factory helpers
	//

	bool createD2DFactory(void)
	{
		return SUCCEEDED(D2D1CreateFactory(
			D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.put()));
	}

	bool createDWriteFactory(void)
	{
		return SUCCEEDED(DWriteCreateFactory(
			DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
			reinterpret_cast<IUnknown**>(m_dwriteFactory.put())));
	}

	bool createWicFactory(void)
	{
		return SUCCEEDED(CoCreateInstance(
			CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory, m_wicFactory.put_void()));
	}

	void releaseResources(void)
	{
		m_surfaces.clear();
		m_wicFactory    = nullptr;
		m_dwriteFactory = nullptr;
		m_d2dFactory    = nullptr;
	}

	//
	// Surface-target helpers
	//

	static bool valid(const D2D1_SIZE_U& size)
	{
		return size.width != 0 && size.height != 0;
	}

	winrt::com_ptr<IWICBitmap> createWicBitmap(const D2D1_SIZE_U& size)
	{
		winrt::com_ptr<IWICBitmap> bitmap;
		m_wicFactory->CreateBitmap(
			size.width, size.height,
			GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnLoad, bitmap.put());
		return bitmap;
	}

	winrt::com_ptr<ID2D1RenderTarget> createWicRenderTarget(IWICBitmap* wicBitmap)
	{
		winrt::com_ptr<ID2D1RenderTarget> renderTarget;
		m_d2dFactory->CreateWicBitmapRenderTarget(
			wicBitmap,
			D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
			renderTarget.put());
		return renderTarget;
	}

	bool buildTarget(const D2D1_SIZE_U& size, SurfaceTarget& target)
	{
		if (!m_wicFactory || !m_d2dFactory) {
			return false;
		}
		auto wicBitmap    = createWicBitmap(size);
		auto renderTarget = wicBitmap ? createWicRenderTarget(wicBitmap.get()) : nullptr;
		if (!renderTarget) {
			return false;
		}
		target = SurfaceTarget{ std::move(wicBitmap), std::move(renderTarget), size };
		return true;
	}

	const SurfaceTarget* find(SurfaceId surface) const
	{
		const auto it = m_surfaces.find(surface);
		return it == m_surfaces.end() ? nullptr : &it->second;
	}

	//
	// IGraphicsResourceKeeper implementation
	//

	bool initialize(void)
	{
		if (m_d2dFactory && m_dwriteFactory && m_wicFactory) {
			return true;
		}
		releaseResources(); // ensure the com_ptr::put() calls below see null
		if (!createD2DFactory() || !createDWriteFactory() || !createWicFactory()) {
			releaseResources();
			return false;
		}
		return true;
	}

	bool deinitialize(void)
	{
		releaseResources();
		return true;
	}

	SurfaceId createSurface(const D2D1_SIZE_U& size)
	{
		SurfaceTarget target {};
		if (!valid(size) || !buildTarget(size, target)) {
			return k_invalidSurface;
		}
		const SurfaceId id = m_nextId++;
		m_surfaces.emplace(id, std::move(target));
		return id;
	}

	bool resizeSurface(SurfaceId surface, const D2D1_SIZE_U& size)
	{
		const auto it = m_surfaces.find(surface);
		if (it == m_surfaces.end() || !valid(size)) {
			return false;
		}
		return buildTarget(size, it->second);
	}

	bool destroySurface(SurfaceId surface)
	{
		return m_surfaces.erase(surface) != 0;
	}

	ID2D1RenderTarget* renderTarget(SurfaceId surface) const
	{
		const SurfaceTarget* target = find(surface);
		return target ? target->renderTarget.get() : nullptr;
	}

	IWICBitmap* wicBitmap(SurfaceId surface) const
	{
		const SurfaceTarget* target = find(surface);
		return target ? target->wicBitmap.get() : nullptr;
	}

	D2D1_SIZE_U surfaceSize(SurfaceId surface) const
	{
		const SurfaceTarget* target = find(surface);
		return target ? target->size : D2D1_SIZE_U{ 0, 0 };
	}

	IDWriteFactory* dwriteFactory(void) const
	{
		return m_dwriteFactory.get();
	}
};

// -----------------------------------------------------------------------------
// OGraphicsResourceKeeper Implementation
// -----------------------------------------------------------------------------
OGraphicsResourceKeeper::OGraphicsResourceKeeper() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OGraphicsResourceKeeper::~OGraphicsResourceKeeper() noexcept = default;
// -----------------------------------------------------------------------------
OGraphicsResourceKeeper& OGraphicsResourceKeeper::getInstance()
{
	static OGraphicsResourceKeeper s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OGraphicsResourceKeeper::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OGraphicsResourceKeeper::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OGraphicsResourceKeeper::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool OGraphicsResourceKeeper::initialize(void)
{
	return m_pImpl->initialize();
}
// -----------------------------------------------------------------------------
bool OGraphicsResourceKeeper::deinitialize(void)
{
	return m_pImpl->deinitialize();
}
// -----------------------------------------------------------------------------
SurfaceId OGraphicsResourceKeeper::createSurface(const D2D1_SIZE_U& size)
{
	return m_pImpl->createSurface(size);
}
// -----------------------------------------------------------------------------
bool OGraphicsResourceKeeper::resizeSurface(SurfaceId surface, const D2D1_SIZE_U& size)
{
	return m_pImpl->resizeSurface(surface, size);
}
// -----------------------------------------------------------------------------
bool OGraphicsResourceKeeper::destroySurface(SurfaceId surface)
{
	return m_pImpl->destroySurface(surface);
}
// -----------------------------------------------------------------------------
ID2D1RenderTarget* OGraphicsResourceKeeper::renderTarget(SurfaceId surface) const
{
	return m_pImpl->renderTarget(surface);
}
// -----------------------------------------------------------------------------
IWICBitmap* OGraphicsResourceKeeper::wicBitmap(SurfaceId surface) const
{
	return m_pImpl->wicBitmap(surface);
}
// -----------------------------------------------------------------------------
D2D1_SIZE_U OGraphicsResourceKeeper::surfaceSize(SurfaceId surface) const
{
	return m_pImpl->surfaceSize(surface);
}
// -----------------------------------------------------------------------------
IDWriteFactory* OGraphicsResourceKeeper::dwriteFactory(void) const
{
	return m_pImpl->dwriteFactory();
}
// -----------------------------------------------------------------------------
