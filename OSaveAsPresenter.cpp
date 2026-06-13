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
#include "OSaveAsPresenter.hpp"
#include "ImageFormats.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <filesystem>

using namespace talay::Save;
using anka::Core::Object::ObjectIdentity;
using anka::Graphics::Conversion::ConversionRequest;
using anka::Graphics::Conversion::IImageConverter;
using anka::Graphics::Conversion::WritableFormat;
using anka::Graphics::Effect::IPixelEffect;

// -----------------------------------------------------------------------------
// OSaveAsPresenter::Impl Definition
// -----------------------------------------------------------------------------
struct OSaveAsPresenter::Impl
{
	ObjectIdentity m_identity {L"SaveAsPresenterObject"};

	IImageConverter& m_converter;

	explicit Impl(IImageConverter& converter) :
		m_converter {converter}
	{}

	// Exactly the formats the encoder chain can write, drawn from the same
	// list the rest of Talay opens — never a curated guess that can drift.
	std::vector<WritableFormat> formats(void) const
	{
		return m_converter.writableFormats(talay::Formats::supportedImageFormats());
	}

	// The picker hands back one full path; the converter wants it as intent
	// (directory + name + extension), so split it here — no caller ever
	// spells a path by hand.
	static ConversionRequest makeRequest(const std::wstring& sourcePath, const std::wstring& destinationPath)
	{
		const std::filesystem::path destination {destinationPath};

		return {.sourcePath      = sourcePath,
		        .outputDirectory = destination.parent_path().wstring(),
		        .outputName      = destination.stem().wstring(),
		        .extension       = destination.extension().wstring()};
	}

	bool saveAs(const std::wstring& sourcePath, const std::wstring& destinationPath) const
	{
		return m_converter.convert(makeRequest(sourcePath, destinationPath)).success;
	}

	bool saveAsWithEffect(const std::wstring& sourcePath, const std::wstring& destinationPath,
	                      const IPixelEffect& effect) const
	{
		return m_converter.convertWithEffect(makeRequest(sourcePath, destinationPath), effect).success;
	}
};

// -----------------------------------------------------------------------------
// OSaveAsPresenter Implementation
// -----------------------------------------------------------------------------
OSaveAsPresenter::OSaveAsPresenter(IImageConverter& converter) :
	m_pImpl {std::make_unique<Impl>(converter)}
{}
// -----------------------------------------------------------------------------
OSaveAsPresenter::~OSaveAsPresenter() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OSaveAsPresenter::getName(void) const
{
	return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OSaveAsPresenter::setName(const std::wstring& name)
{
	m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSaveAsPresenter::getId(void) const
{
	return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
std::vector<WritableFormat> OSaveAsPresenter::formats(void) const
{
	return m_pImpl->formats();
}
// -----------------------------------------------------------------------------
bool OSaveAsPresenter::saveAs(const std::wstring& sourcePath,
                              const std::wstring& destinationPath) const
{
	return m_pImpl->saveAs(sourcePath, destinationPath);
}
// -----------------------------------------------------------------------------
bool OSaveAsPresenter::saveAsWithEffect(const std::wstring& sourcePath,
                                        const std::wstring& destinationPath,
                                        const IPixelEffect& effect) const
{
	return m_pImpl->saveAsWithEffect(sourcePath, destinationPath, effect);
}
// -----------------------------------------------------------------------------
