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
#include "OImageMetadataReader.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/System/Time/UnixTime.hpp"

#include <wincodec.h>
#include <winrt/base.h>

#include <cstdint>
#include <cwctype>
#include <format>
#include <memory>
#include <string>

using namespace anka::Graphics::Image;
using namespace anka::Core::Object;


// Platform plumbing, file-local so neither COM nor the Win32 file API leaks past
// this translation unit.
namespace
{
	// -------------------------------------------------------------------------
	// WIC helpers — image geometry, depth, DPI and container format
	// -------------------------------------------------------------------------

	winrt::com_ptr<IWICImagingFactory> createFactory(void)
	{
		winrt::com_ptr<IWICImagingFactory> factory;
		CoCreateInstance(
			CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
			IID_IWICImagingFactory, factory.put_void());
		return factory;
	}

	// Null when WIC has no codec for this file — metadata stays partial (file-system
	// facts still fill in) rather than failing the whole read.
	winrt::com_ptr<IWICBitmapDecoder> openDecoder(IWICImagingFactory* factory, const std::wstring& path)
	{
		winrt::com_ptr<IWICBitmapDecoder> decoder;
		factory->CreateDecoderFromFilename(
			path.c_str(), nullptr, GENERIC_READ,
			WICDecodeMetadataCacheOnDemand, decoder.put());
		return decoder;
	}

	winrt::com_ptr<IWICBitmapFrameDecode> firstFrame(IWICBitmapDecoder* decoder)
	{
		winrt::com_ptr<IWICBitmapFrameDecode> frame;
		decoder->GetFrame(0, frame.put());
		return frame;
	}

	// Maps the well-known container GUIDs to friendly names; nullptr falls the
	// caller through to the file-extension guess (Open/Closed for new formats).
	const wchar_t* knownFormat(const GUID& container)
	{
		if (container == GUID_ContainerFormatPng)  return L"PNG";
		if (container == GUID_ContainerFormatJpeg) return L"JPEG";
		if (container == GUID_ContainerFormatGif)  return L"GIF";
		if (container == GUID_ContainerFormatBmp)  return L"BMP";
		if (container == GUID_ContainerFormatTiff) return L"TIFF";
		if (container == GUID_ContainerFormatIco)  return L"ICO";
		if (container == GUID_ContainerFormatWmp)  return L"HD Photo";
		return nullptr;
	}

	std::wstring extensionUpper(const std::wstring& path)
	{
		const auto dot = path.find_last_of(L'.');
		if (dot == std::wstring::npos) {
			return {};
		}

		std::wstring ext = path.substr(dot + 1);
		for (auto& ch : ext) {
			ch = static_cast<wchar_t>(std::towupper(ch));
		}
		return ext;
	}

	std::wstring formatName(IWICBitmapDecoder* decoder, const std::wstring& path)
	{
		GUID container {};
		decoder->GetContainerFormat(&container);

		const wchar_t* known = knownFormat(container);
		return known ? known : extensionUpper(path);
	}

	const wchar_t* channelName(UINT channels)
	{
		switch (channels) {
			case 1:  return L"Grayscale";
			case 3:  return L"RGB";
			case 4:  return L"RGBA";
			default: return L"Indexed";
		}
	}

	void fillGeometry(IWICBitmapFrameDecode* frame, ImageMetadata& meta)
	{
		UINT width = 0, height = 0;
		frame->GetSize(&width, &height);
		meta.width  = width;
		meta.height = height;

		frame->GetResolution(&meta.dpiX, &meta.dpiY);
	}

	winrt::com_ptr<IWICPixelFormatInfo> pixelFormatInfo(IWICImagingFactory* factory, IWICBitmapFrameDecode* frame)
	{
		WICPixelFormatGUID pixelFormat {};
		if (FAILED(frame->GetPixelFormat(&pixelFormat))) {
			return nullptr;
		}

		winrt::com_ptr<IWICComponentInfo> component;
		if (FAILED(factory->CreateComponentInfo(pixelFormat, component.put()))) {
			return nullptr;
		}
		return component.try_as<IWICPixelFormatInfo>();
	}

	void fillPixelFormat(IWICImagingFactory* factory, IWICBitmapFrameDecode* frame, ImageMetadata& meta)
	{
		const auto info = pixelFormatInfo(factory, frame);
		if (!info) {
			return;
		}

		UINT bitsPerPixel = 0, channels = 0;
		info->GetBitsPerPixel(&bitsPerPixel);
		info->GetChannelCount(&channels);

		meta.bitDepth     = bitsPerPixel;
		meta.channelCount = channels;
		meta.pixelFormat  = channelName(channels);
	}

	// An embedded colour context means a tagged ICC profile; otherwise the file is
	// interpreted as sRGB, the web/Windows default.
	void fillColorProfile(IWICBitmapFrameDecode* frame, ImageMetadata& meta)
	{
		UINT count = 0;
		frame->GetColorContexts(0, nullptr, &count);
		meta.colorProfile = count > 0 ? L"Embedded ICC" : L"sRGB";
	}

	void fillFrameCount(IWICBitmapDecoder* decoder, ImageMetadata& meta)
	{
		UINT frames = 0;
		if (SUCCEEDED(decoder->GetFrameCount(&frames)) && frames > 0) {
			meta.frameCount = frames;
		}
	}

	void readImageFacts(const std::wstring& path, ImageMetadata& meta)
	{
		const auto factory = createFactory();
		const auto decoder = factory ? openDecoder(factory.get(), path) : nullptr;
		const auto frame   = decoder ? firstFrame(decoder.get()) : nullptr;
		if (!frame) {
			return;
		}

		meta.format = formatName(decoder.get(), path);
		fillFrameCount(decoder.get(), meta);
		fillGeometry(frame.get(), meta);
		fillPixelFormat(factory.get(), frame.get(), meta);
		fillColorProfile(frame.get(), meta);
	}

	// -------------------------------------------------------------------------
	// Win32 file-system helpers — on-disk size and timestamps
	// -------------------------------------------------------------------------

	std::uint64_t combineSize(DWORD high, DWORD low)
	{
		ULARGE_INTEGER size {};
		size.HighPart = high;
		size.LowPart  = low;
		return size.QuadPart;
	}

	// Converts a UTC FILETIME to a locale-neutral local "YYYY-MM-DD HH:MM" — done
	// here (not in the view) because turning file timestamps into civil time is a
	// file-system concern; an empty string signals an unreadable stamp, which the
	// view renders as its placeholder.
	std::wstring formatFileTime(const FILETIME& fileTime)
	{
		FILETIME local {};
		SYSTEMTIME civil {};
		if (!FileTimeToLocalFileTime(&fileTime, &local) || !FileTimeToSystemTime(&local, &civil)) {
			return {};
		}

		return std::format(L"{:04}-{:02}-{:02} {:02}:{:02}",
			civil.wYear, civil.wMonth, civil.wDay, civil.wHour, civil.wMinute);
	}

	void readFileFacts(const std::wstring& path, ImageMetadata& meta)
	{
		WIN32_FILE_ATTRIBUTE_DATA data {};
		if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data)) {
			return;
		}

		meta.fileSizeBytes   = combineSize(data.nFileSizeHigh, data.nFileSizeLow);
		meta.created         = formatFileTime(data.ftCreationTime);
		meta.modified        = formatFileTime(data.ftLastWriteTime);
		meta.modifiedUnixUtc = anka::System::Time::toUnixSeconds(data.ftLastWriteTime);
	}

	std::wstring baseName(const std::wstring& path)
	{
		const auto slash = path.find_last_of(L"\\/");
		return slash == std::wstring::npos ? path : path.substr(slash + 1);
	}
}


// -----------------------------------------------------------------------------
// OImageMetadataReader::Impl Definition
// -----------------------------------------------------------------------------
struct OImageMetadataReader::Impl
{
	ObjectIdentity m_identity {L"ImageMetadataReaderObject"};

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
	// IImageMetadataReader implementation
	//

	// Compose the two independent sources: image facts (WIC) and file facts
	// (Win32). Either may no-op on failure, leaving its fields at their defaults,
	// so a missing codec still yields a name/size/date and vice versa.
	ImageMetadata read(const std::wstring& path) const
	{
		ImageMetadata meta;
		meta.filePath = path;
		meta.fileName = baseName(path);

		readImageFacts(path, meta);
		readFileFacts(path, meta);
		return meta;
	}
};

// -----------------------------------------------------------------------------
// OImageMetadataReader Implementation
// -----------------------------------------------------------------------------
OImageMetadataReader::OImageMetadataReader() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OImageMetadataReader::~OImageMetadataReader() noexcept = default;
// -----------------------------------------------------------------------------
OImageMetadataReader& OImageMetadataReader::getInstance()
{
	static OImageMetadataReader s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OImageMetadataReader::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OImageMetadataReader::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImageMetadataReader::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
ImageMetadata OImageMetadataReader::read(const std::wstring& path) const
{
	return m_pImpl->read(path);
}
// -----------------------------------------------------------------------------
