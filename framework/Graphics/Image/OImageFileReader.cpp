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
#include "OImageFileReader.hpp"
#include "OWicImageDecoder.hpp"
#include "OFreeImageDecoder.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <cstdint>
#include <memory>
#include <vector>

using namespace anka::Graphics::Image;
using namespace anka::Core::Object;


// -----------------------------------------------------------------------------
// OImageFileReader::Impl Definition
// -----------------------------------------------------------------------------
struct OImageFileReader::Impl
{
	ObjectIdentity                              m_identity {L"ImageFileReaderObject"};

	// The decode chain, in priority order: WIC first, FreeImage as the fallback.
	// New backends (HEIC, AVIF, ...) slot in here as extra strategies — read()
	// below never changes (Open/Closed).
	std::vector<std::unique_ptr<IImageDecoder>> m_decoders;

	Impl()
	{
		m_decoders.push_back(std::make_unique<OWicImageDecoder>());
		m_decoders.push_back(std::make_unique<OFreeImageDecoder>());
	}

	//
	// IObject delegation
	//

	const std::wstring& getObjectName(void) const
	{
		return m_identity.name();
	}

	void setObjectName(const std::wstring& name)
	{
		m_identity.setName(name);
	}

	std::uint64_t getObjectId(void) const
	{
		return m_identity.id();
	}

	//
	// IImageFileReader implementation
	//

	// Walk the chain and stop at the first backend that decodes the file.
	ImageSequence readSequence(const std::wstring& path) const
	{
		ImageSequence sequence;
		for (const auto& decoder : m_decoders) {
			if (decoder->decode(path, sequence)) {
				break;
			}
		}
		return sequence;
	}

	// Still-only convenience: the first frame of whatever the chain produced.
	DecodedImage read(const std::wstring& path) const
	{
		const ImageSequence sequence = readSequence(path);
		return sequence.empty() ? DecodedImage{} : sequence.frame(0).image();
	}
};

// -----------------------------------------------------------------------------
// OImageFileReader Implementation
// -----------------------------------------------------------------------------
OImageFileReader::OImageFileReader() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OImageFileReader::~OImageFileReader() noexcept = default;
// -----------------------------------------------------------------------------
OImageFileReader& OImageFileReader::getInstance()
{
	static OImageFileReader s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OImageFileReader::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OImageFileReader::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImageFileReader::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
ImageSequence OImageFileReader::readSequence(const std::wstring& path) const
{
	return m_pImpl->readSequence(path);
}
// -----------------------------------------------------------------------------
DecodedImage OImageFileReader::read(const std::wstring& path) const
{
	return m_pImpl->read(path);
}
// -----------------------------------------------------------------------------
