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
#include "OPrintPresenter.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <filesystem>
#include <utility>

using namespace talay::Print;
using anka::Core::Object::ObjectIdentity;
using anka::Graphics::Image::DecodedImage;
using anka::Graphics::Image::IImageFileReader;
using anka::Graphics::Print::IImagePrinter;
using anka::Graphics::Print::PrintOutcome;

// -----------------------------------------------------------------------------
// OPrintPresenter::Impl Definition
// -----------------------------------------------------------------------------
struct OPrintPresenter::Impl
{
	ObjectIdentity m_identity {L"PrintPresenterObject"};

	IImageFileReader& m_reader;
	IImagePrinter&    m_printer;

	Impl(IImageFileReader& reader, IImagePrinter& printer) :
		m_reader  {reader},
		m_printer {printer}
	{}

	// The spooler queue shows the document name to the user; the bare file
	// name is the one they will recognise there.
	static std::wstring documentName(const std::wstring& path)
	{
		return std::filesystem::path {path}.filename().wstring();
	}

	// Print from a fresh, full-resolution decode of the file — never from the
	// screen bitmap — so paper quality is independent of window size or zoom.
	// An animation contributes its first frame (that is what read() yields).
	PrintOutcome printFile(const std::wstring& path, HWND owner) const
	{
		DecodedImage image {m_reader.read(path)};
		if (image.empty())
			return PrintOutcome::Failed;

		return m_printer.print({.image = std::move(image), .documentName = documentName(path), .owner = owner});
	}
};

// -----------------------------------------------------------------------------
// OPrintPresenter Implementation
// -----------------------------------------------------------------------------
OPrintPresenter::OPrintPresenter(IImageFileReader& reader, IImagePrinter& printer) :
	m_pImpl {std::make_unique<Impl>(reader, printer)}
{}
// -----------------------------------------------------------------------------
OPrintPresenter::~OPrintPresenter() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OPrintPresenter::getName(void) const
{
	return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OPrintPresenter::setName(const std::wstring& name)
{
	m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OPrintPresenter::getId(void) const
{
	return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
PrintOutcome OPrintPresenter::printFile(const std::wstring& path, HWND owner) const
{
	return m_pImpl->printFile(path, owner);
}
// -----------------------------------------------------------------------------
