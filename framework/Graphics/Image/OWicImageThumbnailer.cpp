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
#include "OWicImageThumbnailer.hpp"
#include "OImageFileReader.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <wincodec.h>
#include <winrt/base.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#pragma comment(lib, "ole32.lib") // CoCreateInstance

using namespace anka::Graphics::Image;
using namespace anka::Core::Object;


// WIC plumbing, file-local so COM never leaks beyond this translation unit. It
// mirrors the still-decode path of OWicImageDecoder but adds a scaling step, so a
// folder of large photos costs only thumbnail-sized buffers.
namespace
{
	constexpr std::uint32_t k_bytesPerPixel = 4;   // 32bpp PBGRA

	struct Extent
	{
		UINT width  {0};
		UINT height {0};
	};

	winrt::com_ptr<IWICImagingFactory> createFactory(void)
	{
		winrt::com_ptr<IWICImagingFactory> factory;
		CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
		                 IID_IWICImagingFactory, factory.put_void());
		return factory;
	}

	// Null when WIC has no codec for this file — the cue to fall back to the full
	// reader chain (which also fields the FreeImage-only formats).
	winrt::com_ptr<IWICBitmapDecoder> openDecoder(IWICImagingFactory* factory, const std::wstring& path)
	{
		winrt::com_ptr<IWICBitmapDecoder> decoder;
		factory->CreateDecoderFromFilename(path.c_str(), nullptr, GENERIC_READ,
		                                   WICDecodeMetadataCacheOnDemand, decoder.put());
		return decoder;
	}

	winrt::com_ptr<IWICBitmapFrameDecode> firstFrame(IWICBitmapDecoder* decoder)
	{
		winrt::com_ptr<IWICBitmapFrameDecode> frame;
		decoder->GetFrame(0, frame.put());
		return frame;
	}

	// The largest size that fits within a 'maxEdge' square while keeping the aspect
	// ratio; images already small enough are left at their own size (no upscaling).
	Extent fitWithin(Extent source, std::uint32_t maxEdge)
	{
		const UINT longest = (std::max)(source.width, source.height);
		if (longest == 0 || longest <= maxEdge) {
			return source;
		}

		const double scale = static_cast<double>(maxEdge) / longest;
		return { (std::max)(1u, static_cast<UINT>(std::lround(source.width * scale))),
		         (std::max)(1u, static_cast<UINT>(std::lround(source.height * scale))) };
	}

	// The frame itself when no scaling is needed, otherwise a Fant-resampled scaler
	// over it — either way an IWICBitmapSource the converter can consume uniformly.
	winrt::com_ptr<IWICBitmapSource> scaleToFit(IWICImagingFactory* factory,
	                                            const winrt::com_ptr<IWICBitmapFrameDecode>& frame, Extent target)
	{
		Extent actual;
		frame->GetSize(&actual.width, &actual.height);
		if (target.width == actual.width && target.height == actual.height) {
			return frame.as<IWICBitmapSource>();
		}

		winrt::com_ptr<IWICBitmapScaler> scaler;
		factory->CreateBitmapScaler(scaler.put());
		scaler->Initialize(frame.get(), target.width, target.height, WICBitmapInterpolationModeFant);
		return scaler.as<IWICBitmapSource>();
	}

	// Normalise to premultiplied BGRA, the one layout the rest of the pipeline (and
	// WriteableBitmap) consumes.
	winrt::com_ptr<IWICBitmapSource> toPremultipliedBgra(IWICImagingFactory* factory, IWICBitmapSource* source)
	{
		winrt::com_ptr<IWICFormatConverter> converter;
		if (FAILED(factory->CreateFormatConverter(converter.put()))) {
			return nullptr;
		}

		converter->Initialize(source, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
		                       nullptr, 0.0, WICBitmapPaletteTypeCustom);
		return converter.as<IWICBitmapSource>();
	}

	DecodedImage copyOut(IWICBitmapSource* source)
	{
		Extent size;
		if (FAILED(source->GetSize(&size.width, &size.height)) || size.width == 0 || size.height == 0) {
			return {};
		}

		const std::uint32_t stride = size.width * k_bytesPerPixel;
		std::vector<std::byte> pixels(static_cast<std::size_t>(stride) * size.height);
		if (FAILED(source->CopyPixels(nullptr, stride, static_cast<UINT>(pixels.size()),
		                              reinterpret_cast<BYTE*>(pixels.data())))) {
			return {};
		}

		return DecodedImage{ std::move(pixels), size.width, size.height, stride };
	}

	// Decode → scale → normalise; empty on any WIC miss so the caller can fall back.
	DecodedImage decodeScaled(const std::wstring& path, std::uint32_t maxEdge)
	{
		const auto factory = createFactory();
		const auto decoder = factory ? openDecoder(factory.get(), path) : nullptr;
		const auto frame   = decoder ? firstFrame(decoder.get()) : nullptr;
		if (!frame) {
			return {};
		}

		Extent source;
		frame->GetSize(&source.width, &source.height);
		const auto fitted = scaleToFit(factory.get(), frame, fitWithin(source, maxEdge));
		const auto bgra   = fitted ? toPremultipliedBgra(factory.get(), fitted.get()) : nullptr;
		return bgra ? copyOut(bgra.get()) : DecodedImage{};
	}
}


// -----------------------------------------------------------------------------
// OWicImageThumbnailer::Impl Definition
// -----------------------------------------------------------------------------
struct OWicImageThumbnailer::Impl
{
	ObjectIdentity m_identity {L"WicImageThumbnailerObject"};

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

	// Prefer the cheap scaled WIC decode; fall back to the full reader chain so the
	// reel still shows formats only FreeImage can open (the RAW family, ...).
	DecodedImage thumbnail(const std::wstring& path, std::uint32_t maxEdge) const
	{
		DecodedImage scaled = decodeScaled(path, maxEdge);
		return scaled.empty() ? OImageFileReader::getInstance().read(path) : scaled;
	}
};

// -----------------------------------------------------------------------------
// OWicImageThumbnailer Implementation
// -----------------------------------------------------------------------------
OWicImageThumbnailer::OWicImageThumbnailer() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OWicImageThumbnailer::~OWicImageThumbnailer() noexcept = default;
// -----------------------------------------------------------------------------
OWicImageThumbnailer& OWicImageThumbnailer::getInstance()
{
	static OWicImageThumbnailer s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OWicImageThumbnailer::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OWicImageThumbnailer::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OWicImageThumbnailer::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
DecodedImage OWicImageThumbnailer::thumbnail(const std::wstring& path, std::uint32_t maxEdge) const
{
	return m_pImpl->thumbnail(path, maxEdge);
}
// -----------------------------------------------------------------------------
