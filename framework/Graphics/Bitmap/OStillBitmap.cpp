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
#include "OStillBitmap.hpp"

#include <d2d1.h>
#include <winrt/base.h>

using namespace anka::Graphics::Bitmap;


// -----------------------------------------------------------------------------
// OStillBitmap::Impl Definition
// -----------------------------------------------------------------------------
struct OStillBitmap::Impl
{
	winrt::com_ptr<ID2D1Bitmap> m_frame;
	D2D1_SIZE_U                 m_size;

	Impl(ID2D1Bitmap* frame, const D2D1_SIZE_U& size) :
		m_size {size}
	{
		m_frame.copy_from(frame);
	}
};

// -----------------------------------------------------------------------------
// OStillBitmap Implementation
// -----------------------------------------------------------------------------
OStillBitmap::OStillBitmap(ID2D1Bitmap* frame, const D2D1_SIZE_U& size) :
	m_pImpl {std::make_unique<Impl>(frame, size)}
{}
// -----------------------------------------------------------------------------
OStillBitmap::~OStillBitmap() noexcept = default;
// -----------------------------------------------------------------------------
bool OStillBitmap::isAnimated(void) const
{
	return false;
}
// -----------------------------------------------------------------------------
ID2D1Bitmap* OStillBitmap::currentFrame(void) const
{
	return m_pImpl->m_frame.get();
}
// -----------------------------------------------------------------------------
D2D1_SIZE_U OStillBitmap::pixelSize(void) const
{
	return m_pImpl->m_size;
}
// -----------------------------------------------------------------------------
std::chrono::milliseconds OStillBitmap::currentDelay(void) const
{
	return std::chrono::milliseconds{0};
}
// -----------------------------------------------------------------------------
bool OStillBitmap::advance(void)
{
	return false;
}
// -----------------------------------------------------------------------------
void OStillBitmap::reset(void)
{}
// -----------------------------------------------------------------------------
