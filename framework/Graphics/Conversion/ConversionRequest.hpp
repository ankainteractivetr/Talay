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

#ifndef ANKA_GRAPHICS_CONVERSION_CONVERSIONREQUEST_HPP
#define ANKA_GRAPHICS_CONVERSION_CONVERSIONREQUEST_HPP

#include <string>

namespace anka::Graphics::Conversion
{
	// What to convert and where to put it — the single, self-describing input to
	// IImageConverter. Keeping every parameter the user chose in one value type (a
	// Parameter Object) means the convert() signature never grows as more options
	// arrive, and the request is trivially copyable across the UI/worker boundary.
	//
	// 'extension' is the target format with no leading dot (e.g. L"png"); the
	// converter composes the output path as outputDirectory / (outputName + "." +
	// extension), so the caller supplies intent, not a hand-built path.
	struct ConversionRequest
	{
		std::wstring sourcePath;        // file to read and convert
		std::wstring outputDirectory;   // folder the result is written into
		std::wstring outputName;        // result file name, without extension
		std::wstring extension;         // target format, no leading dot (e.g. L"png")
	};

	// The outcome of a conversion: whether it succeeded and, on success, the full
	// path that was written. An empty outputPath mirrors the failure case so the
	// caller can report success/failure without a second out-parameter.
	struct ConversionResult
	{
		bool         success {false};
		std::wstring outputPath;
	};

	// One selectable target format for the conversion UI: the dot-less, lower-case
	// 'extension' that goes straight into a ConversionRequest, plus a 'label' (the
	// same extension upper-cased) for display. The converter hands these out so the
	// window lists exactly the formats that can actually be written, never a curated
	// guess that can drift from the codec's real capabilities.
	struct WritableFormat
	{
		std::wstring extension;   // target, no leading dot, lower-case (e.g. L"png")
		std::wstring label;       // display text, upper-case (e.g. L"PNG")
	};
}

#endif // ANKA_GRAPHICS_CONVERSION_CONVERSIONREQUEST_HPP
