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

#ifndef ANKA_GRAPHICS_PRINT_PRINTREQUEST_HPP
#define ANKA_GRAPHICS_PRINT_PRINTREQUEST_HPP

#include "framework/Graphics/Image/DecodedImage.hpp"
#include "framework/System/Windows/windows.hpp"

#include <string>

namespace anka::Graphics::Print
{
	// What to print and how to label it — the single, self-describing input to
	// IImagePrinter (a Parameter Object, like ConversionRequest). The image is
	// held by value so the caller can move freshly decoded pixels in and the
	// request stays valid for the whole job, independent of any UI state
	// changing underneath it.
	//
	// 'documentName' is what the spooler queue shows the user (typically the
	// bare file name); 'owner' anchors the system print dialog.
	struct PrintRequest
	{
		Image::DecodedImage image;
		std::wstring        documentName;
		HWND                owner {nullptr};
	};

	// How a print attempt ended. Cancelled is the user closing the dialog —
	// normal flow, never an error — so callers can stay quiet about it and
	// surface only Failed.
	enum class PrintOutcome
	{
		Printed,
		Cancelled,
		Failed
	};
}

#endif // ANKA_GRAPHICS_PRINT_PRINTREQUEST_HPP
