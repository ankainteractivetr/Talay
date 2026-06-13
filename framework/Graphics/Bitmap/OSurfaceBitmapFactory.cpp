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
#include "OSurfaceBitmapFactory.hpp"
#include "OBitmapFactory.hpp"
#include "OContinuousBitmap.hpp"
#include "OStillBitmap.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "framework/Graphics/Image/DecodedImage.hpp"
#include "framework/Graphics/Image/ImageFrame.hpp"

#include <d2d1.h>
#include <winrt/base.h>

#include <cstddef>

using namespace anka::Graphics;
using namespace anka::Graphics::Bitmap;
using namespace anka::Graphics::Image;
using namespace anka::Core::Object;


// -----------------------------------------------------------------------------
// OSurfaceBitmapFactory::Impl Definition
// -----------------------------------------------------------------------------
struct OSurfaceBitmapFactory::Impl
{
	ObjectIdentity m_identity {L"SurfaceBitmapFactoryObject"};

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
	// ISurfaceBitmapFactory implementation
	//

	static D2D1_SIZE_U sizeOf(const DecodedImage& image)
	{
		return D2D1::SizeU(image.width(), image.height());
	}

	// Upload one frame's premultiplied-BGRA pixels into a Direct2D bitmap bound to
	// the surface, reusing the lower-level factory (composition, not inheritance).
	winrt::com_ptr<ID2D1Bitmap> upload(SurfaceId surface, const DecodedImage& image)
	{
		winrt::com_ptr<ID2D1Bitmap> bitmap;
		OBitmapFactory::getInstance().createFromMemory(
			surface, image.pixels(), image.stride(), sizeOf(image), bitmap.put());
		return bitmap;
	}

	std::unique_ptr<ISurfaceBitmap> buildStill(SurfaceId surface, const ImageSequence& sequence)
	{
		const DecodedImage& image = sequence.frame(0).image();
		const auto bitmap = upload(surface, image);
		if (!bitmap) {
			return nullptr;
		}
		return std::make_unique<OStillBitmap>(bitmap.get(), sizeOf(image));
	}

	bool appendFrame(OContinuousBitmap& target, SurfaceId surface, const ImageFrame& frame)
	{
		const auto bitmap = upload(surface, frame.image());
		if (!bitmap) {
			return false;
		}
		target.addFrame(bitmap.get(), frame.delay());
		return true;
	}

	std::unique_ptr<ISurfaceBitmap> buildContinuous(SurfaceId surface, const ImageSequence& sequence)
	{
		auto target = std::make_unique<OContinuousBitmap>(sizeOf(sequence.frame(0).image()), sequence.loopCount());
		for (std::size_t index = 0; index < sequence.frameCount(); ++index) {
			if (!appendFrame(*target, surface, sequence.frame(index))) return nullptr;
		}
		return target;
	}

	std::unique_ptr<ISurfaceBitmap> create(SurfaceId surface, const ImageSequence& sequence)
	{
		if (sequence.empty()) {
			return nullptr;
		}
		return sequence.isAnimated() ? buildContinuous(surface, sequence) : buildStill(surface, sequence);
	}
};

// -----------------------------------------------------------------------------
// OSurfaceBitmapFactory Implementation
// -----------------------------------------------------------------------------
OSurfaceBitmapFactory::OSurfaceBitmapFactory() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OSurfaceBitmapFactory::~OSurfaceBitmapFactory() noexcept = default;
// -----------------------------------------------------------------------------
OSurfaceBitmapFactory& OSurfaceBitmapFactory::getInstance()
{
	static OSurfaceBitmapFactory s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OSurfaceBitmapFactory::getName(void) const
{
	return m_pImpl->getObjectName();
}
// -----------------------------------------------------------------------------
void OSurfaceBitmapFactory::setName(const std::wstring& name)
{
	m_pImpl->setObjectName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSurfaceBitmapFactory::getId(void) const
{
	return m_pImpl->getObjectId();
}
// -----------------------------------------------------------------------------
std::unique_ptr<ISurfaceBitmap> OSurfaceBitmapFactory::create(SurfaceId surface, const Image::ImageSequence& sequence)
{
	return m_pImpl->create(surface, sequence);
}
// -----------------------------------------------------------------------------
