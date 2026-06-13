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

#ifndef TALAY_SAVE_ISAVEASPRESENTER_HPP
#define TALAY_SAVE_ISAVEASPRESENTER_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Conversion/ConversionRequest.hpp"

#include <string>
#include <vector>

// Forward-declared: only referenced here; the implementation that bakes the
// effect into the written copy pulls in the full type.
namespace anka::Graphics::Effect { class IPixelEffect; }

namespace talay::Save
{
	/*
	* Abstract Save-As Presenter Interface (MVP)
	*
	* The logic half of File - Save As: it turns "save the open image as ..."
	* into a single-file conversion - decode at full resolution, encode into
	* the format named by the destination's extension - and answers whether
	* the file was written. The window stays a dumb control surface: it picks
	* the destination and maps the outcome onto a dialog.
	*
	*/
	class ISaveAsPresenter : public anka::Core::Object::IObject
	{
		public:

			virtual ~ISaveAsPresenter() noexcept override = default;

			// Every format the save dialog may offer, verified against the
			// encoder chain: the label for display, the extension for the path.
			virtual std::vector<anka::Graphics::Conversion::WritableFormat> formats(void) const = 0;

			// Convert the image at 'sourcePath' into 'destinationPath'; the
			// target format is the destination's extension. True = written.
			virtual bool saveAs(const std::wstring& sourcePath,
			                    const std::wstring& destinationPath) const = 0;

			// As saveAs, but bakes 'effect' - the view effect the user is
			// looking at - into the written copy. The source never changes.
			virtual bool saveAsWithEffect(const std::wstring& sourcePath,
			                              const std::wstring& destinationPath,
			                              const anka::Graphics::Effect::IPixelEffect& effect) const = 0;
	};
}

#endif // TALAY_SAVE_ISAVEASPRESENTER_HPP
