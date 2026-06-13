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

#ifndef ANKA_GRAPHICS_PRINT_IIMAGEPRINTER_HPP
#define ANKA_GRAPHICS_PRINT_IIMAGEPRINTER_HPP

#include "PrintRequest.hpp"
#include "framework/Core/Object/IObject.hpp"

namespace anka::Graphics::Print
{
	/*
	* Abstract Image Printer Interface (Facade)
	*
	* One call hides the whole Windows print pipeline: the printer-picker
	* dialog, page setup, rasterisation and the spooler handshake. Callers
	* hand over decoded pixels and learn only how the attempt ended, so no
	* GDI type or print API detail ever leaks above this seam.
	*
	*/
	class IImagePrinter : public anka::Core::Object::IObject
	{
		public:

			virtual ~IImagePrinter() noexcept override = default;

			// Shows the system print dialog anchored to request.owner, then
			// prints request.image fitted and centred on a single page.
			virtual PrintOutcome print(PrintRequest request) const = 0;
	};
}

#endif // ANKA_GRAPHICS_PRINT_IIMAGEPRINTER_HPP
