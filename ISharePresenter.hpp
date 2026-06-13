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

#ifndef TALAY_SHARE_ISHAREPRESENTER_HPP
#define TALAY_SHARE_ISHAREPRESENTER_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/System/Windows/windows.hpp"

#include <string>

// Forward-declared: only referenced here; the implementation that bakes the
// effect into the shared copy pulls in the full type.
namespace anka::Graphics::Effect { class IPixelEffect; }

namespace talay::Share
{
	/*
	* Abstract Share Presenter Interface (MVP)
	*
	* The logic half of File - Share: it turns "share this file" into a
	* clipboard publication - decode at full resolution, hand the pixels to
	* the system clipboard - and answers whether the image is now pastable
	* in other applications. The window stays a dumb control surface: it
	* forwards the current path and maps the outcome onto a dialog.
	*
	*/
	class ISharePresenter : public anka::Core::Object::IObject
	{
		public:

			virtual ~ISharePresenter() noexcept override = default;

			// Decode the image at 'path' and copy it onto the clipboard on
			// behalf of 'owner'. True means ready to paste elsewhere; false
			// is the only outcome worth a message.
			virtual bool shareFile(const std::wstring& path, HWND owner) const = 0;

			// As shareFile, but bakes 'effect' — the view effect the user is
			// looking at — into the published copy. The file and its decode
			// stay untouched: the effect exists only on the clipboard.
			virtual bool shareFileWithEffect(const std::wstring& path, HWND owner,
			                                 const anka::Graphics::Effect::IPixelEffect& effect) const = 0;
	};
}

#endif // TALAY_SHARE_ISHAREPRESENTER_HPP
