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
#include "OContinuousBitmap.hpp"

#include <d2d1.h>
#include <winrt/base.h>

#include <chrono>
#include <cstdint>
#include <vector>

using namespace anka::Graphics::Bitmap;


// -----------------------------------------------------------------------------
// OContinuousBitmap::Impl Definition
// -----------------------------------------------------------------------------
struct OContinuousBitmap::Impl
{
	struct Frame
	{
		winrt::com_ptr<ID2D1Bitmap> bitmap;
		std::chrono::milliseconds   delay;
	};

	std::vector<Frame> m_frames;
	D2D1_SIZE_U        m_size;
	std::uint32_t      m_loopCount;          // 0 == loop forever
	std::size_t        m_index     {0};
	std::uint32_t      m_loopsDone {0};
	bool               m_finished  {false};

	Impl(const D2D1_SIZE_U& size, std::uint32_t loopCount) :
		m_size {size},
		m_loopCount {loopCount}
	{}

	void addFrame(ID2D1Bitmap* bitmap, std::chrono::milliseconds delay)
	{
		Frame frame;
		frame.bitmap.copy_from(bitmap);
		frame.delay = delay;
		m_frames.push_back(std::move(frame));
	}

	// Returning to the start completes one play; stop on the last frame once the
	// requested number of plays is reached (a finite loopCount).
	bool wrap(void)
	{
		if (m_loopCount != 0 && ++m_loopsDone >= m_loopCount) {
			m_finished = true;
			return false;
		}
		m_index = 0;
		return true;
	}

	bool advance(void)
	{
		if (m_finished || m_frames.size() <= 1) {
			return false;
		}
		return (m_index + 1 < m_frames.size()) ? (++m_index, true) : wrap();
	}

	void reset(void)
	{
		m_index     = 0;
		m_loopsDone = 0;
		m_finished  = false;
	}
};

// -----------------------------------------------------------------------------
// OContinuousBitmap Implementation
// -----------------------------------------------------------------------------
OContinuousBitmap::OContinuousBitmap(const D2D1_SIZE_U& size, std::uint32_t loopCount) :
	m_pImpl {std::make_unique<Impl>(size, loopCount)}
{}
// -----------------------------------------------------------------------------
OContinuousBitmap::~OContinuousBitmap() noexcept = default;
// -----------------------------------------------------------------------------
void OContinuousBitmap::addFrame(ID2D1Bitmap* frame, std::chrono::milliseconds delay)
{
	m_pImpl->addFrame(frame, delay);
}
// -----------------------------------------------------------------------------
bool OContinuousBitmap::isAnimated(void) const
{
	return m_pImpl->m_frames.size() > 1;
}
// -----------------------------------------------------------------------------
ID2D1Bitmap* OContinuousBitmap::currentFrame(void) const
{
	return m_pImpl->m_frames.empty() ? nullptr : m_pImpl->m_frames[m_pImpl->m_index].bitmap.get();
}
// -----------------------------------------------------------------------------
D2D1_SIZE_U OContinuousBitmap::pixelSize(void) const
{
	return m_pImpl->m_size;
}
// -----------------------------------------------------------------------------
std::chrono::milliseconds OContinuousBitmap::currentDelay(void) const
{
	return m_pImpl->m_frames.empty() ? std::chrono::milliseconds{0} : m_pImpl->m_frames[m_pImpl->m_index].delay;
}
// -----------------------------------------------------------------------------
bool OContinuousBitmap::advance(void)
{
	return m_pImpl->advance();
}
// -----------------------------------------------------------------------------
void OContinuousBitmap::reset(void)
{
	m_pImpl->reset();
}
// -----------------------------------------------------------------------------
