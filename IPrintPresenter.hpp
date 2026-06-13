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

#ifndef TALAY_PRINT_IPRINTPRESENTER_HPP
#define TALAY_PRINT_IPRINTPRESENTER_HPP

#include "framework/Core/Object/IObject.hpp"
#include "framework/Graphics/Print/PrintRequest.hpp"

#include <string>

namespace talay::Print
{
	/*
	* Abstract Print Presenter Interface (MVP)
	*
	* The logic half of File › Print: it turns "print this file" into a print
	* job — decode at full resolution, name the job after the file, run the
	* system print flow — and answers with how it ended. The window stays a
	* dumb control surface: it forwards the current path and maps the outcome
	* onto a dialog.
	*
	*/
	class IPrintPresenter : public anka::Core::Object::IObject
	{
		public:

			virtual ~IPrintPresenter() noexcept override = default;

			// Decode the image at 'path' and send it through the print flow
			// anchored to 'owner'. Cancelled reports the user's own choice in
			// the printer dialog; Failed is the only outcome worth a message.
			virtual anka::Graphics::Print::PrintOutcome printFile(const std::wstring& path, HWND owner) const = 0;
	};
}

#endif // TALAY_PRINT_IPRINTPRESENTER_HPP
