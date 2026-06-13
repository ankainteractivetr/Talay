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

#ifndef ANKA_GRAPHICS_CONVERSION_IIMAGECONVERTER_HPP
#define ANKA_GRAPHICS_CONVERSION_IIMAGECONVERTER_HPP

#include "ConversionRequest.hpp"
#include "framework/Core/Object/IObject.hpp"

#include <string>
#include <vector>

// Forward-declared: only referenced here; the implementation that bakes the
// effect into the written pixels pulls in the full type.
namespace anka::Graphics::Effect { class IPixelEffect; }

namespace anka::Graphics::Conversion
{
	// Facade over the read → encode pipeline. It hides that a conversion is really
	// "decode the source into the framework's DecodedImage currency, then encode it
	// in the requested format": the window asks for a ConversionRequest to be
	// fulfilled and gets back a ConversionResult, learning nothing about codecs.
	//
	// The single convert() entry point is the whole surface, so the UI depends only
	// on this abstraction (Dependency Inversion) and new output formats are added by
	// extending the encoder chain behind the facade, never by touching callers.
	class IImageConverter : public anka::Core::Object::IObject
	{
		public:

			virtual ~IImageConverter() noexcept override = default;

			// Reads request.sourcePath and writes the converted image; the result
			// reports success and the path that was written.
			virtual ConversionResult convert(const ConversionRequest& request) const = 0;

			// As convert, but bakes 'effect' into the written pixels. The source
			// file and its decode stay untouched, so a read-only view effect can
			// be exported without ever mutating the original.
			virtual ConversionResult convertWithEffect(const ConversionRequest& request,
			                                           const anka::Graphics::Effect::IPixelEffect& effect) const = 0;

			// The subset of 'candidates' the encoder chain can actually write, with
			// the aliases of one codec collapsed to a single entry (".JPG"/".JPEG" →
			// one "JPG"). The order of 'candidates' is preserved, so the UI shows the
			// caller's preferred ordering.
			virtual std::vector<WritableFormat> writableFormats(const std::vector<std::wstring>& candidates) const = 0;
	};
}

#endif // ANKA_GRAPHICS_CONVERSION_IIMAGECONVERTER_HPP
