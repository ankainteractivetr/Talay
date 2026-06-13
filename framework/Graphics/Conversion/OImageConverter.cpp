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
#include "OImageConverter.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Effect/EffectApplication.hpp"
#include "framework/Graphics/Image/DecodedImage.hpp"
#include "framework/Graphics/Image/IImageEncoder.hpp"
#include "framework/Graphics/Image/OFreeImageEncoder.hpp"
#include "framework/Graphics/Image/OImageFileReader.hpp"

#include <algorithm>
#include <cstdint>
#include <cwctype>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_set>
#include <vector>

using namespace anka::Graphics::Conversion;
using namespace anka::Core::Object;
using anka::Graphics::Effect::IPixelEffect;
using anka::Graphics::Image::DecodedImage;
using anka::Graphics::Image::IImageEncoder;
using anka::Graphics::Image::OFreeImageEncoder;
using anka::Graphics::Image::OImageFileReader;

namespace EffectApplication = anka::Graphics::Effect::EffectApplication;

// -----------------------------------------------------------------------------
// Local helpers — pure request → path arithmetic, no I/O.
// -----------------------------------------------------------------------------
namespace
{
	std::wstring stripLeadingDot(const std::wstring& extension)
	{
		return (!extension.empty() && extension.front() == L'.') ? extension.substr(1) : extension;
	}

	// Compose outputDirectory / (outputName + "." + extension) with std::filesystem,
	// so separators and edge cases are handled by the standard library, not by hand.
	std::wstring composeOutputPath(const ConversionRequest& request)
	{
		std::filesystem::path path {request.outputDirectory};
		path /= request.outputName + L"." + stripLeadingDot(request.extension);
		return path.wstring();
	}

	bool requestIsComplete(const ConversionRequest& request)
	{
		return !request.sourcePath.empty()
		    && !request.outputDirectory.empty()
		    && !request.outputName.empty()
		    && !request.extension.empty();
	}

	std::wstring transformedExtension(const std::wstring& extension, std::wint_t (*map)(std::wint_t))
	{
		std::wstring result = stripLeadingDot(extension);
		std::transform(result.begin(), result.end(), result.begin(),
		               [map](wchar_t character) { return static_cast<wchar_t>(map(character)); });
		return result;
	}

	// The two presentations the UI needs of one writable extension: the dot-less
	// lower-case tag a ConversionRequest carries, and its upper-case display label.
	WritableFormat makeWritableFormat(const std::wstring& extension)
	{
		return { transformedExtension(extension, std::towlower), transformedExtension(extension, std::towupper) };
	}
}

// -----------------------------------------------------------------------------
// OImageConverter::Impl Definition
// -----------------------------------------------------------------------------
struct OImageConverter::Impl
{
	ObjectIdentity                              m_identity {L"ImageConverterObject"};

	// The encode chain, in priority order. FreeImage covers every format the UI
	// offers today; new backends (a WIC encoder, an HEIC writer, ...) slot in here
	// as extra strategies — convert() below never changes (Open/Closed).
	std::vector<std::unique_ptr<IImageEncoder>> m_encoders;

	Impl()
	{
		m_encoders.push_back(std::make_unique<OFreeImageEncoder>());
	}

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

	// Try each encoder in turn; succeed at the first that writes the file.
	bool encode(const DecodedImage& image, const std::wstring& path) const
	{
		for (const auto& encoder : m_encoders) {
			if (encoder->encode(image, path)) {
				return true;
			}
		}
		return false;
	}

	// Decode the source into the framework currency, then encode it as requested.
	ConversionResult convert(const ConversionRequest& request) const
	{
		return convertImage(request, nullptr);
	}

	ConversionResult convertWithEffect(const ConversionRequest& request, const IPixelEffect& effect) const
	{
		return convertImage(request, &effect);
	}

	// The one decode -> (optional effect) -> encode pipeline behind both entry
	// points; a null effect means "write the pixels exactly as decoded".
	ConversionResult convertImage(const ConversionRequest& request, const IPixelEffect* effect) const
	{
		if (!requestIsComplete(request)) {
			return {};
		}

		const DecodedImage image = sourceImage(request.sourcePath, effect);
		if (image.empty()) {
			return {};
		}

		return writeImage(image, composeOutputPath(request));
	}

	// The decoded source, with the optional effect baked into a copy — the
	// decode itself is never mutated.
	static DecodedImage sourceImage(const std::wstring& path, const IPixelEffect* effect)
	{
		DecodedImage image = OImageFileReader::getInstance().read(path);
		if (effect != nullptr) {
			return EffectApplication::applied(image, *effect);
		}

		return image;
	}

	ConversionResult writeImage(const DecodedImage& image, const std::wstring& outputPath) const
	{
		const bool written = encode(image, outputPath);
		return {written, written ? outputPath : std::wstring{}};
	}

	// The codec key the first capable encoder would use for 'extension', or empty
	// when no backend in the chain can write that format.
	std::wstring firstEncodableKey(const std::wstring& extension) const
	{
		for (const auto& encoder : m_encoders) {
			const std::wstring key = encoder->encodableFormatKey(extension);
			if (!key.empty()) {
				return key;
			}
		}
		return {};
	}

	// Keep each candidate the chain can write, collapsing every alias of one codec
	// (.JPG/.JPEG, .TIF/.TIFF, ...) to the first that appears in the input.
	std::vector<WritableFormat> writableFormats(const std::vector<std::wstring>& candidates) const
	{
		std::unordered_set<std::wstring> seen;
		std::vector<WritableFormat> formats;
		for (const auto& candidate : candidates) {
			const std::wstring key = firstEncodableKey(candidate);
			if (!key.empty() && seen.insert(key).second) {
				formats.push_back(makeWritableFormat(candidate));
			}
		}
		return formats;
	}
};

// -----------------------------------------------------------------------------
// OImageConverter Implementation
// -----------------------------------------------------------------------------
OImageConverter::OImageConverter() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OImageConverter::~OImageConverter() noexcept = default;
// -----------------------------------------------------------------------------
OImageConverter& OImageConverter::getInstance()
{
	static OImageConverter s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OImageConverter::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OImageConverter::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImageConverter::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
ConversionResult OImageConverter::convert(const ConversionRequest& request) const
{
	return m_pImpl->convert(request);
}
// -----------------------------------------------------------------------------
ConversionResult OImageConverter::convertWithEffect(const ConversionRequest& request,
                                                    const IPixelEffect& effect) const
{
	return m_pImpl->convertWithEffect(request, effect);
}
// -----------------------------------------------------------------------------
std::vector<WritableFormat> OImageConverter::writableFormats(const std::vector<std::wstring>& candidates) const
{
	return m_pImpl->writableFormats(candidates);
}
// -----------------------------------------------------------------------------
