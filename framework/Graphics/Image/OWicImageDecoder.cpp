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
#include "OWicImageDecoder.hpp"

#include <wincodec.h>
#include <winrt/base.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

#pragma comment(lib, "ole32.lib") // PropVariantInit / PropVariantClear

using namespace anka::Graphics::Image;


// WIC plumbing, file-local so it never leaks COM beyond this translation unit.
namespace
{
	constexpr std::uint32_t k_bytesPerPixel  = 4;    // 32bpp PBGRA
	constexpr UINT          k_defaultDelayMs = 100;  // frames with a 0 delay (browser rule)

	struct Extent
	{
		UINT width  {0};
		UINT height {0};
	};

	// How a frame is composited onto the running canvas before the next one — the
	// common vocabulary every animated format is normalised to, so the compositor
	// below stays format-agnostic (GIF disposal codes, APNG dispose_op, ... all map
	// here).
	enum class Disposal
	{
		None,              // leave the canvas as drawn
		RestoreBackground, // clear this frame's rectangle afterwards
		RestorePrevious    // roll the canvas back to before this frame
	};

	// Everything the compositor needs about one frame, independent of the container
	// it came from. Width/height are the sub-frame's own (read from its pixels).
	struct FrameInfo
	{
		UINT                      xCoord {0};
		UINT                      yCoord {0};
		Disposal                  disposal {Disposal::None};
		std::chrono::milliseconds delay {0};
	};


	// -------------------------------------------------------------------------
	// Generic WIC helpers (shared by stills and every animated format)
	// -------------------------------------------------------------------------

	winrt::com_ptr<IWICImagingFactory> createFactory(void)
	{
		winrt::com_ptr<IWICImagingFactory> factory;
		CoCreateInstance(
			CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory, factory.put_void());
		return factory;
	}

	// Null when WIC has no codec for this file (WINCODEC_ERR_COMPONENTNOTFOUND) —
	// the cue for the reader to fall through to the next decoder.
	winrt::com_ptr<IWICBitmapDecoder> openDecoder(IWICImagingFactory* factory, const std::wstring& path)
	{
		winrt::com_ptr<IWICBitmapDecoder> decoder;
		factory->CreateDecoderFromFilename(
			path.c_str(), nullptr, GENERIC_READ,
			WICDecodeMetadataCacheOnDemand, decoder.put());
		return decoder;
	}

	UINT frameCount(IWICBitmapDecoder* decoder)
	{
		UINT count = 0;
		decoder->GetFrameCount(&count);
		return count;
	}

	GUID containerFormat(IWICBitmapDecoder* decoder)
	{
		GUID container {};
		decoder->GetContainerFormat(&container);
		return container;
	}

	winrt::com_ptr<IWICBitmapFrameDecode> getFrame(IWICBitmapDecoder* decoder, UINT index)
	{
		winrt::com_ptr<IWICBitmapFrameDecode> frame;
		decoder->GetFrame(index, frame.put());
		return frame;
	}

	// Normalise any source pixel format to premultiplied BGRA, the one layout
	// Direct2D consumes — so the rest of the pipeline is format-agnostic.
	winrt::com_ptr<IWICFormatConverter> toPremultipliedBgra(IWICImagingFactory* factory, IWICBitmapSource* source)
	{
		winrt::com_ptr<IWICFormatConverter> converter;
		if (FAILED(factory->CreateFormatConverter(converter.put()))) {
			return nullptr;
		}

		converter->Initialize(
			source, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone,
			nullptr, 0.0, WICBitmapPaletteTypeCustom);
		return converter;
	}

	DecodedImage copyOut(IWICBitmapSource* source)
	{
		UINT width = 0;
		UINT height = 0;
		if (FAILED(source->GetSize(&width, &height)) || width == 0 || height == 0) {
			return {};
		}

		const std::uint32_t stride = width * k_bytesPerPixel;
		std::vector<std::byte> pixels(static_cast<std::size_t>(stride) * height);
		if (FAILED(source->CopyPixels(
				nullptr, stride, static_cast<UINT>(pixels.size()),
				reinterpret_cast<BYTE*>(pixels.data())))) {
			return {};
		}

		return DecodedImage{ std::move(pixels), width, height, stride };
	}

	// One frame's pixels in PBGRA — used for both still images and animation frames.
	DecodedImage convertFrame(IWICImagingFactory* factory, IWICBitmapFrameDecode* frame)
	{
		const auto converter = toPremultipliedBgra(factory, frame);
		return converter ? copyOut(converter.get()) : DecodedImage{};
	}

	Extent firstFrameExtent(IWICBitmapDecoder* decoder)
	{
		Extent extent;
		const auto frame = getFrame(decoder, 0);
		if (frame) {
			frame->GetSize(&extent.width, &extent.height);
		}
		return extent;
	}

	winrt::com_ptr<IWICMetadataQueryReader> frameReader(IWICBitmapFrameDecode* frame)
	{
		winrt::com_ptr<IWICMetadataQueryReader> reader;
		frame->GetMetadataQueryReader(reader.put());
		return reader;
	}

	winrt::com_ptr<IWICMetadataQueryReader> decoderReader(IWICBitmapDecoder* decoder)
	{
		winrt::com_ptr<IWICMetadataQueryReader> reader;
		decoder->GetMetadataQueryReader(reader.put());
		return reader;
	}


	// -------------------------------------------------------------------------
	// Metadata reading (PROPVARIANT helpers, shared by the profiles)
	// -------------------------------------------------------------------------

	// Pull an unsigned integer out of a metadata PROPVARIANT regardless of its
	// concrete unsigned width (these are stored as UI1/UI2).
	UINT propToUint(const PROPVARIANT& value, UINT fallback)
	{
		switch (value.vt) {
			case VT_UI1: return value.bVal;
			case VT_UI2: return value.uiVal;
			case VT_UI4: return value.ulVal;
			default:     return fallback;
		}
	}

	// Read a named metadata value, yielding 'fallback' for absent keys, so callers
	// stay branch-free on presence (a null reader counts as absent).
	UINT readUint(IWICMetadataQueryReader* reader, LPCWSTR name, UINT fallback)
	{
		PROPVARIANT value;
		PropVariantInit(&value);
		const HRESULT hr = reader ? reader->GetMetadataByName(name, &value) : E_POINTER;
		const UINT result = SUCCEEDED(hr) ? propToUint(value, fallback) : fallback;
		PropVariantClear(&value);
		return result;
	}


	// -------------------------------------------------------------------------
	// Animation-format strategy (the extension point for continuous formats)
	//
	// Each profile teaches the generic compositor how to read ONE animated
	// container's frame geometry, timing, disposal and loop count from WIC
	// metadata. Adding a new continuous format (animated WebP, APNG, ...) is a
	// new profile plus one entry in selectProfile — the compositor and decode
	// path never change (Open/Closed). Kept file-local so <wincodec.h> stays
	// sealed inside this translation unit.
	// -------------------------------------------------------------------------

	class IAnimationProfile
	{
		public:

			virtual ~IAnimationProfile() noexcept = default;

			virtual bool         handles(const GUID& container) const = 0;
			virtual Extent       canvasSize(IWICMetadataQueryReader* reader, Extent firstFrame) const = 0;
			virtual FrameInfo    frameInfo(IWICMetadataQueryReader* reader) const = 0;
			virtual std::uint32_t loopCount(IWICMetadataQueryReader* reader) const = 0;
	};

	// GIF: canvas = logical screen descriptor; per-frame rect + disposal + delay
	// from the image descriptor and graphic-control extension; loops from the
	// Netscape application extension.
	class GifAnimationProfile final : public IAnimationProfile
	{
		public:

			bool handles(const GUID& container) const override
			{
				return container == GUID_ContainerFormatGif;
			}

			Extent canvasSize(IWICMetadataQueryReader* reader, Extent firstFrame) const override
			{
				return {
					readUint(reader, L"/logscrdesc/Width",  firstFrame.width),
					readUint(reader, L"/logscrdesc/Height", firstFrame.height) };
			}

			FrameInfo frameInfo(IWICMetadataQueryReader* reader) const override
			{
				return {
					readUint(reader, L"/imgdesc/Left", 0),
					readUint(reader, L"/imgdesc/Top", 0),
					toDisposal(readUint(reader, L"/grctlext/Disposal", 0)),
					readDelay(reader) };
			}

			std::uint32_t loopCount(IWICMetadataQueryReader* reader) const override
			{
				return readLoopCount(reader);
			}

		private:

			static Disposal toDisposal(UINT code)
			{
				if (code == 2) return Disposal::RestoreBackground;
				if (code == 3) return Disposal::RestorePrevious;
				return Disposal::None;
			}

			static std::chrono::milliseconds readDelay(IWICMetadataQueryReader* reader)
			{
				const UINT centiseconds = readUint(reader, L"/grctlext/Delay", 0);
				const UINT milliseconds = centiseconds ? centiseconds * 10 : k_defaultDelayMs;
				return std::chrono::milliseconds{ milliseconds };
			}

			// Plays from the Netscape application extension; 0 == loop forever.
			static std::uint32_t loopsFromAppExt(const PROPVARIANT& value)
			{
				const auto& bytes = value.caub;
				if (value.vt != (VT_UI1 | VT_VECTOR) || bytes.cElems < 4) return ImageSequence::k_loopForever;
				if (bytes.pElems[0] != 3 || bytes.pElems[1] != 1)         return ImageSequence::k_loopForever;
				return static_cast<std::uint32_t>(bytes.pElems[2] | (bytes.pElems[3] << 8));
			}

			static std::uint32_t readLoopCount(IWICMetadataQueryReader* reader)
			{
				PROPVARIANT value;
				PropVariantInit(&value);
				const HRESULT hr = reader ? reader->GetMetadataByName(L"/appext/Data", &value) : E_POINTER;
				const std::uint32_t loops = SUCCEEDED(hr) ? loopsFromAppExt(value) : ImageSequence::k_loopForever;
				PropVariantClear(&value);
				return loops;
			}
	};

	// The registry: the first profile whose container matches wins; null means
	// "not a continuous format" (so multi-page TIFF / multi-image ICO stay still).
	const IAnimationProfile* selectProfile(const GUID& container)
	{
		static const GifAnimationProfile s_gif;
		static const IAnimationProfile* const s_profiles[] = { &s_gif };

		for (const auto* profile : s_profiles) {
			if (profile->handles(container)) return profile;
		}
		return nullptr;
	}


	// -------------------------------------------------------------------------
	// Generic frame compositor (format-agnostic, driven by FrameInfo)
	// -------------------------------------------------------------------------

	// The full-canvas accumulator: one PBGRA, top-down buffer the size of the
	// logical screen. Frames are composited onto it, then snapshotted.
	struct Canvas
	{
		std::vector<std::byte> pixels;
		UINT                   width  {0};
		UINT                   height {0};

		bool empty(void) const { return pixels.empty(); }

		std::byte* at(UINT xCoord, UINT yCoord)
		{
			return pixels.data() + (static_cast<std::size_t>(yCoord) * width + xCoord) * k_bytesPerPixel;
		}
	};

	Canvas makeCanvas(Extent size)
	{
		Canvas canvas;
		if (size.width && size.height) {
			canvas = { std::vector<std::byte>(static_cast<std::size_t>(size.width) * size.height * k_bytesPerPixel), size.width, size.height };
		}
		return canvas;
	}

	// How many columns/rows of a sub-frame at 'origin' actually land inside 'limit'.
	UINT visibleExtent(UINT dimension, UINT origin, UINT limit)
	{
		return origin < limit ? (std::min)(dimension, limit - origin) : 0u;
	}

	// Premultiplied source-over of one BGRA pixel: out = src + dst * (1 - srcAlpha).
	void blendPixel(std::byte* dst, const std::byte* src)
	{
		const UINT alpha = std::to_integer<UINT>(src[3]);
		const UINT inverse = 255 - alpha;
		for (int channel = 0; channel < 4; ++channel) {
			const UINT s = std::to_integer<UINT>(src[channel]);
			const UINT d = std::to_integer<UINT>(dst[channel]);
			dst[channel] = static_cast<std::byte>(s + (d * inverse + 127) / 255);
		}
	}

	void compositeOver(Canvas& canvas, const DecodedImage& sub, UINT xCoord, UINT yCoord)
	{
		const UINT width  = visibleExtent(sub.width(),  xCoord, canvas.width);
		const UINT height = visibleExtent(sub.height(), yCoord, canvas.height);
		const auto* src = static_cast<const std::byte*>(sub.pixels());
		for (UINT row = 0; row < height; ++row) {
			const std::byte* srcRow = src + static_cast<std::size_t>(row) * sub.stride();
			std::byte*       dstRow = canvas.at(xCoord, yCoord + row);
			for (UINT col = 0; col < width; ++col) {
				blendPixel(dstRow + col * k_bytesPerPixel, srcRow + col * k_bytesPerPixel);
			}
		}
	}

	DecodedImage snapshot(const Canvas& canvas)
	{
		const std::uint32_t stride = canvas.width * k_bytesPerPixel;
		return DecodedImage{ canvas.pixels, canvas.width, canvas.height, stride };
	}

	void clearRect(Canvas& canvas, const DecodedImage& sub, const FrameInfo& info)
	{
		const UINT width  = visibleExtent(sub.width(),  info.xCoord, canvas.width);
		const UINT height = visibleExtent(sub.height(), info.yCoord, canvas.height);
		for (UINT row = 0; row < height; ++row) {
			std::byte* dstRow = canvas.at(info.xCoord, info.yCoord + row);
			std::fill(dstRow, dstRow + static_cast<std::size_t>(width) * k_bytesPerPixel, std::byte{0});
		}
	}

	// Prepare the canvas for the *next* frame per this frame's disposal.
	void applyDisposal(Canvas& canvas, Canvas& saved, const DecodedImage& sub, const FrameInfo& info)
	{
		if (info.disposal == Disposal::RestoreBackground) {
			clearRect(canvas, sub, info);
		} else if (info.disposal == Disposal::RestorePrevious && !saved.empty()) {
			canvas.pixels = std::move(saved.pixels);
		}
	}

	// Paint the frame onto the canvas, snapshot it, then run its disposal — saving
	// a rollback copy beforehand only when the frame asks to be restored.
	void blitAndDispose(Canvas& canvas, const DecodedImage& sub, const FrameInfo& info, ImageSequence& sequence)
	{
		Canvas saved = (info.disposal == Disposal::RestorePrevious) ? canvas : Canvas{};
		compositeOver(canvas, sub, info.xCoord, info.yCoord);
		sequence.addFrame({ snapshot(canvas), info.delay });
		applyDisposal(canvas, saved, sub, info);
	}

	bool composeFrame(IWICImagingFactory* factory, IWICBitmapDecoder* decoder, const IAnimationProfile& profile, UINT index, Canvas& canvas, ImageSequence& sequence)
	{
		const auto frame = getFrame(decoder, index);
		const DecodedImage sub = frame ? convertFrame(factory, frame.get()) : DecodedImage{};
		if (sub.empty()) {
			return false;
		}

		blitAndDispose(canvas, sub, profile.frameInfo(frameReader(frame.get()).get()), sequence);
		return true;
	}


	// -------------------------------------------------------------------------
	// Decode paths
	// -------------------------------------------------------------------------

	bool decodeStill(IWICImagingFactory* factory, IWICBitmapDecoder* decoder, ImageSequence& sequence)
	{
		const auto frame = getFrame(decoder, 0);
		DecodedImage image = frame ? convertFrame(factory, frame.get()) : DecodedImage{};
		if (image.empty()) {
			return false;
		}

		sequence.addFrame({ std::move(image), std::chrono::milliseconds{0} });
		return true;
	}

	bool decodeAnimated(IWICImagingFactory* factory, IWICBitmapDecoder* decoder, const IAnimationProfile& profile, UINT frames, ImageSequence& sequence)
	{
		const auto reader = decoderReader(decoder);
		Canvas canvas = makeCanvas(profile.canvasSize(reader.get(), firstFrameExtent(decoder)));
		if (canvas.empty()) {
			return false;
		}

		for (UINT index = 0; index < frames; ++index) {
			if (!composeFrame(factory, decoder, profile, index, canvas, sequence)) return false;
		}
		sequence.setLoopCount(profile.loopCount(reader.get()));
		return !sequence.empty();
	}
}


// -----------------------------------------------------------------------------
// OWicImageDecoder Implementation
// -----------------------------------------------------------------------------
bool OWicImageDecoder::decode(const std::wstring& path, ImageSequence& sequence) const
{
	const auto factory = createFactory();
	const auto decoder = factory ? openDecoder(factory.get(), path) : nullptr;
	const UINT frames  = decoder ? frameCount(decoder.get()) : 0;
	if (frames == 0) {
		return false;
	}

	const IAnimationProfile* profile = frames > 1 ? selectProfile(containerFormat(decoder.get())) : nullptr;
	return profile
		? decodeAnimated(factory.get(), decoder.get(), *profile, frames, sequence)
		: decodeStill(factory.get(), decoder.get(), sequence);
}
// -----------------------------------------------------------------------------
