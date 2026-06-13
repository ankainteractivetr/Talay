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
#include "ORenderer.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Resource/OGraphicsResourceKeeper.hpp"

#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <winrt/base.h>

#include <cstdint>

using namespace anka::Graphics;
using namespace anka::Graphics::Renderer;
using namespace anka::Graphics::Resource;
using namespace anka::Core::Object;


// -----------------------------------------------------------------------------
// ORenderer::Impl Definition
// -----------------------------------------------------------------------------
struct ORenderer::Impl
{
	// On-demand renderer. It owns no graphics resources: the durable state lives
	// in the graphics resource keeper, which the renderer borrows from. The only
	// state here is m_active, the target selected for the current frame, valid
	// strictly between beginDraw() and endDraw().
	ObjectIdentity     m_identity {L"Direct2DRendererObject"};
	ID2D1RenderTarget* m_active {nullptr}; // non-owning, valid within a frame

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
	// Collaborators
	//

	IGraphicsResourceKeeper& keeper(void) const
	{
		return OGraphicsResourceKeeper::getInstance();
	}

	//
	// Draw helpers (operate on the active target)
	//

	winrt::com_ptr<IDWriteTextFormat> createTextFormat(std::int32_t size) const
	{
		winrt::com_ptr<IDWriteTextFormat> textFormat;
		IDWriteFactory* factory = keeper().dwriteFactory();
		if (factory) {
			factory->CreateTextFormat(
				L"Segoe UI", nullptr,
				DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
				static_cast<FLOAT>(size), L"en-us", textFormat.put());
		}
		return textFormat;
	}

	winrt::com_ptr<ID2D1SolidColorBrush> createBrush(const D2D1_COLOR_F& color) const
	{
		winrt::com_ptr<ID2D1SolidColorBrush> brush;
		m_active->CreateSolidColorBrush(color, brush.put());
		return brush;
	}

	D2D1_RECT_F textLayoutRect(std::int32_t xCoord, std::int32_t yCoord) const
	{
		const D2D1_SIZE_F size = m_active->GetSize();
		return D2D1::RectF(
			static_cast<FLOAT>(xCoord), static_cast<FLOAT>(yCoord),
			size.width, size.height);
	}

	//
	// IRenderer implementation
	//

	bool initialize(void)
	{
		return keeper().initialize();
	}

	bool deinitialize(void)
	{
		m_active = nullptr;
		return keeper().deinitialize();
	}

	bool beginDraw(SurfaceId surface)
	{
		m_active = keeper().renderTarget(surface);
		if (!m_active) {
			return false;
		}
		m_active->BeginDraw();
		m_active->SetTransform(D2D1::Matrix3x2F::Identity());
		return true;
	}

	bool clear(const D2D1_COLOR_F& color)
	{
		if (!m_active) {
			return false;
		}
		m_active->Clear(color);
		return true;
	}

	bool setTransform(const D2D1_MATRIX_3X2_F& transform)
	{
		if (!m_active) {
			return false;
		}
		m_active->SetTransform(transform);
		return true;
	}

	bool drawBitmap(const D2D1_RECT_F& source, const D2D1_RECT_F& destination, ID2D1Bitmap* bitmap)
	{
		if (!m_active || bitmap == nullptr) {
			return false;
		}
		m_active->DrawBitmap(
			bitmap, &destination, 1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &source);
		return true;
	}

	bool drawText(const std::wstring& text, const std::int32_t xCoord, const std::int32_t yCoord, const std::int32_t size, const D2D1_COLOR_F& color)
	{
		if (!m_active) {
			return false;
		}
		const auto textFormat = createTextFormat(size);
		const auto brush      = createBrush(color);
		if (!textFormat || !brush) {
			return false;
		}
		m_active->DrawText(
			text.c_str(), static_cast<UINT32>(text.size()),
			textFormat.get(), textLayoutRect(xCoord, yCoord), brush.get());
		return true;
	}

	bool endDraw(void)
	{
		if (!m_active) {
			return false;
		}
		const HRESULT hr = m_active->EndDraw();
		m_active = nullptr;
		return SUCCEEDED(hr);
	}

	bool copyPixels(SurfaceId surface, std::uint8_t* destination, std::uint32_t stride, std::uint32_t size)
	{
		IWICBitmap* wicBitmap = destination ? keeper().wicBitmap(surface) : nullptr;
		if (!wicBitmap) {
			return false;
		}
		const D2D1_SIZE_U dim = keeper().surfaceSize(surface);
		const WICRect rect { 0, 0, static_cast<INT>(dim.width), static_cast<INT>(dim.height) };
		return SUCCEEDED(wicBitmap->CopyPixels(&rect, stride, size, destination));
	}
};

// -----------------------------------------------------------------------------
// ORenderer Implementation
// -----------------------------------------------------------------------------
ORenderer::ORenderer() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
ORenderer::~ORenderer() noexcept = default;
// -----------------------------------------------------------------------------
ORenderer& ORenderer::getInstance()
{
	static ORenderer s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& ORenderer::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void ORenderer::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t ORenderer::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
bool ORenderer::initialize(void)
{
	return m_pImpl->initialize();
}
// -----------------------------------------------------------------------------
bool ORenderer::deinitialize(void)
{
	return m_pImpl->deinitialize();
}
// -----------------------------------------------------------------------------
bool ORenderer::beginDraw(SurfaceId surface)
{
	return m_pImpl->beginDraw(surface);
}
// -----------------------------------------------------------------------------
bool ORenderer::setTransform(const D2D1_MATRIX_3X2_F& transform)
{
	return m_pImpl->setTransform(transform);
}
// -----------------------------------------------------------------------------
bool ORenderer::clear(const D2D1_COLOR_F& color)
{
	return m_pImpl->clear(color);
}
// -----------------------------------------------------------------------------
bool ORenderer::endDraw(void)
{
	return m_pImpl->endDraw();
}
// -----------------------------------------------------------------------------
bool ORenderer::drawBitmap(const D2D1_RECT_F& source, const D2D1_RECT_F& destination, ID2D1Bitmap* bitmap)
{
	return m_pImpl->drawBitmap(source, destination, bitmap);
}
// -----------------------------------------------------------------------------
bool ORenderer::drawText(const std::wstring& text, const std::int32_t xCoord, const std::int32_t yCoord, const std::int32_t size, const D2D1_COLOR_F& color)
{
	return m_pImpl->drawText(text, xCoord, yCoord, size, color);
}
// -----------------------------------------------------------------------------
bool ORenderer::copyPixels(SurfaceId surface, std::uint8_t* destination, std::uint32_t stride, std::uint32_t size)
{
	return m_pImpl->copyPixels(surface, destination, stride, size);
}
// -----------------------------------------------------------------------------
