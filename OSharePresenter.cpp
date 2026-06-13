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
#include "OSharePresenter.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Effect/EffectApplication.hpp"

using namespace talay::Share;
using anka::Core::Object::ObjectIdentity;
using anka::Graphics::Effect::IPixelEffect;
using anka::Graphics::Image::DecodedImage;
using anka::Graphics::Image::IImageFileReader;
using anka::System::Clipboard::IImageClipboard;

namespace EffectApplication = anka::Graphics::Effect::EffectApplication;

// -----------------------------------------------------------------------------
// OSharePresenter::Impl Definition
// -----------------------------------------------------------------------------
struct OSharePresenter::Impl
{
	ObjectIdentity m_identity {L"SharePresenterObject"};

	IImageFileReader& m_reader;
	IImageClipboard&  m_clipboard;

	Impl(IImageFileReader& reader, IImageClipboard& clipboard) :
		m_reader    {reader},
		m_clipboard {clipboard}
	{}

	// Share from a fresh, full-resolution decode of the file - never from the
	// screen bitmap - so the pasted image is independent of window size or
	// zoom. An animation contributes its first frame (that is what read() yields).
	bool shareFile(const std::wstring& path, HWND owner) const
	{
		const DecodedImage image {m_reader.read(path)};
		if (image.empty())
			return false;

		return m_clipboard.copyImage(image, owner);
	}

	// The same fresh decode, with the user's view effect baked into the
	// published copy only - the file on disk never changes.
	bool shareFileWithEffect(const std::wstring& path, HWND owner, const IPixelEffect& effect) const
	{
		const DecodedImage image {m_reader.read(path)};
		if (image.empty())
			return false;

		return m_clipboard.copyImage(EffectApplication::applied(image, effect), owner);
	}
};

// -----------------------------------------------------------------------------
// OSharePresenter Implementation
// -----------------------------------------------------------------------------
OSharePresenter::OSharePresenter(IImageFileReader& reader, IImageClipboard& clipboard) :
	m_pImpl {std::make_unique<Impl>(reader, clipboard)}
{}
// -----------------------------------------------------------------------------
OSharePresenter::~OSharePresenter() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OSharePresenter::getName(void) const
{
	return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OSharePresenter::setName(const std::wstring& name)
{
	m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSharePresenter::getId(void) const
{
	return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
bool OSharePresenter::shareFile(const std::wstring& path, HWND owner) const
{
	return m_pImpl->shareFile(path, owner);
}
// -----------------------------------------------------------------------------
bool OSharePresenter::shareFileWithEffect(const std::wstring& path, HWND owner,
                                          const IPixelEffect& effect) const
{
	return m_pImpl->shareFileWithEffect(path, owner, effect);
}
// -----------------------------------------------------------------------------
