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
#include "OImagePrinter.hpp"
#include "PageFitting.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <commdlg.h>

#include <cstdint>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#pragma comment(lib, "Comdlg32.lib")   // PrintDlgExW
#pragma comment(lib, "Gdi32.lib")      // StartDoc / StretchDIBits / DeleteDC

using namespace anka::Graphics::Print;
using anka::Core::Object::ObjectIdentity;
using anka::Graphics::Image::DecodedImage;

namespace
{
	// Deleters so every native handle rides in a unique_ptr and is released on
	// all paths — early returns and exceptions included — instead of by hand.
	struct DeviceContextDeleter
	{
		void operator()(HDC device) const
		{
			DeleteDC(device);
		}
	};
	using DeviceContext = std::unique_ptr<std::remove_pointer_t<HDC>, DeviceContextDeleter>;

	struct GlobalMemoryDeleter
	{
		void operator()(HGLOBAL memory) const
		{
			GlobalFree(memory);
		}
	};
	using GlobalMemory = std::unique_ptr<void, GlobalMemoryDeleter>;

	// RAII frame around one spooler job: the destructor always closes it, and
	// a failure in between turns the close into AbortDoc, so a half-drawn
	// document never leaves the machine.
	class DocumentScope
	{
		public:

			DocumentScope(HDC device, const std::wstring& name) :
				m_device {device}
			{
				const DOCINFOW info {.cbSize = static_cast<int>(sizeof(DOCINFOW)), .lpszDocName = name.c_str()};
				m_open = StartDocW(device, &info) > 0;
			}

			DocumentScope(const DocumentScope& scope) = delete;
			DocumentScope& operator=(const DocumentScope& scope) = delete;

			~DocumentScope()
			{
				if (!m_open)
					return;

				if (m_failed)
					AbortDoc(m_device);
				else
					EndDoc(m_device);
			}

			bool isOpen(void) const
			{
				return m_open;
			}

			void markFailed(void)
			{
				m_failed = true;
			}

		private:

			HDC  m_device;
			bool m_open   {false};
			bool m_failed {false};
	};
}

// -----------------------------------------------------------------------------
// OImagePrinter::Impl Definition
// -----------------------------------------------------------------------------
struct OImagePrinter::Impl
{
	ObjectIdentity m_identity {L"ImagePrinterObject"};

	// The user's answer to the printer dialog: whether to go ahead at all and,
	// only when 'verdict' is Printed, the owned device to print on.
	struct PrinterChoice
	{
		PrintOutcome  verdict {PrintOutcome::Cancelled};
		DeviceContext device;
	};

	// All flags up front so the dialog needs no page-range plumbing: a single
	// page, no selection, and the driver handles copies/collation itself.
	static PRINTDLGEXW makePrinterDialog(HWND owner)
	{
		PRINTDLGEXW dialog {};
		dialog.lStructSize = static_cast<DWORD>(sizeof(PRINTDLGEXW));
		dialog.hwndOwner   = owner;
		dialog.Flags       = PD_RETURNDC | PD_NOPAGENUMS | PD_NOSELECTION | PD_NOCURRENTPAGE | PD_USEDEVMODECOPIESANDCOLLATE;
		dialog.nCopies     = 1;
		dialog.nStartPage  = START_PAGE_GENERAL;
		return dialog;
	}

	// Cancel and "Apply, then close" both mean "no job now": only an explicit
	// Print click with a usable device context goes ahead.
	static PrinterChoice choosePrinter(HWND owner)
	{
		PRINTDLGEXW dialog {makePrinterDialog(owner)};
		if (FAILED(PrintDlgExW(&dialog)))
			return {.verdict = PrintOutcome::Failed};

		const GlobalMemory mode {dialog.hDevMode};
		const GlobalMemory names {dialog.hDevNames};
		DeviceContext device {dialog.hDC};

		if (dialog.dwResultAction != PD_RESULT_PRINT || !device)
			return {.verdict = PrintOutcome::Cancelled};

		return {.verdict = PrintOutcome::Printed, .device = std::move(device)};
	}

	static PageMetrics readPageMetrics(HDC device)
	{
		return {.width  = GetDeviceCaps(device, HORZRES),
		        .height = GetDeviceCaps(device, VERTRES),
		        .dpiX   = GetDeviceCaps(device, LOGPIXELSX),
		        .dpiY   = GetDeviceCaps(device, LOGPIXELSY)};
	}

	static BITMAPINFO topDownBitmapInfo(std::uint32_t width, std::uint32_t height)
	{
		BITMAPINFO info {};
		info.bmiHeader.biSize        = static_cast<DWORD>(sizeof(BITMAPINFOHEADER));
		info.bmiHeader.biWidth       = static_cast<LONG>(width);
		info.bmiHeader.biHeight      = -static_cast<LONG>(height);   // negative height: top-down rows
		info.bmiHeader.biPlanes      = 1;
		info.bmiHeader.biBitCount    = 32;
		info.bmiHeader.biCompression = BI_RGB;
		return info;
	}

	// Premultiplied source over a white sheet: out = src + (255 - alpha) per
	// channel. The premultiply invariant (channel <= alpha) keeps every sum
	// inside a byte, so no clamping is needed.
	static void flattenRow(std::uint8_t* target, const std::uint8_t* source, std::uint32_t width)
	{
		for (std::uint32_t pixel {0}; pixel < width; ++pixel, target += 4, source += 4) {
			const std::uint8_t sheet {static_cast<std::uint8_t>(255 - source[3])};
			target[0] = static_cast<std::uint8_t>(source[0] + sheet);
			target[1] = static_cast<std::uint8_t>(source[1] + sheet);
			target[2] = static_cast<std::uint8_t>(source[2] + sheet);
			target[3] = 255;
		}
	}

	// Paper is opaque, so transparency is resolved against white before the
	// pixels reach the driver. Repacking to tight width*4 rows comes free —
	// exactly the row layout StretchDIBits expects for a 32-bit DIB.
	static std::vector<std::byte> flattenOverWhite(const DecodedImage& image)
	{
		std::vector<std::byte> flat(std::size_t {image.width()} * image.height() * 4);
		const auto* source {static_cast<const std::uint8_t*>(image.pixels())};
		auto* target {reinterpret_cast<std::uint8_t*>(flat.data())};

		for (std::uint32_t row {0}; row < image.height(); ++row)
			flattenRow(target + std::size_t {4} * row * image.width(),
			           source + std::size_t {row} * image.stride(), image.width());

		return flat;
	}

	// HALFTONE asks the driver for its best resample (a photo nearly always
	// shrinks onto paper); the frame keeps the physical aspect on the page.
	static bool drawImage(HDC device, const DecodedImage& image)
	{
		const std::vector<std::byte> pixels {flattenOverWhite(image)};
		const PageFrame frame {fitCentered(image.width(), image.height(), readPageMetrics(device))};
		SetStretchBltMode(device, HALFTONE);
		SetBrushOrgEx(device, 0, 0, nullptr);

		const BITMAPINFO info {topDownBitmapInfo(image.width(), image.height())};
		const int copied {StretchDIBits(device, frame.x, frame.y, frame.width, frame.height,
		                                0, 0, static_cast<int>(image.width()), static_cast<int>(image.height()),
		                                pixels.data(), &info, DIB_RGB_COLORS, SRCCOPY)};
		return copied > 0;
	}

	// One image, one page: open the job, draw, close. DocumentScope turns any
	// failure below into AbortDoc, so the spooler never keeps a broken job.
	static PrintOutcome printPage(HDC device, const PrintRequest& request)
	{
		DocumentScope document {device, request.documentName};
		if (!document.isOpen() || StartPage(device) <= 0) {
			document.markFailed();
			return PrintOutcome::Failed;
		}

		const bool drawn {drawImage(device, request.image)};
		if (EndPage(device) <= 0 || !drawn) {
			document.markFailed();
			return PrintOutcome::Failed;
		}

		return PrintOutcome::Printed;
	}

	PrintOutcome print(PrintRequest request) const
	{
		if (request.image.empty())
			return PrintOutcome::Failed;

		const PrinterChoice choice {choosePrinter(request.owner)};
		if (choice.verdict != PrintOutcome::Printed)
			return choice.verdict;

		return printPage(choice.device.get(), request);
	}
};

// -----------------------------------------------------------------------------
// OImagePrinter Implementation
// -----------------------------------------------------------------------------
OImagePrinter::OImagePrinter() :
	m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OImagePrinter::~OImagePrinter() noexcept = default;
// -----------------------------------------------------------------------------
OImagePrinter& OImagePrinter::getInstance()
{
	static OImagePrinter s_instance;
	return s_instance;
}
// -----------------------------------------------------------------------------
const std::wstring& OImagePrinter::getName(void) const
{
	return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OImagePrinter::setName(const std::wstring& name)
{
	m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OImagePrinter::getId(void) const
{
	return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
PrintOutcome OImagePrinter::print(PrintRequest request) const
{
	return m_pImpl->print(std::move(request));
}
// -----------------------------------------------------------------------------
