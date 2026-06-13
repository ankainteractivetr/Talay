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

#ifndef ANKA_GRAPHICS_PRINT_OIMAGEPRINTER_HPP
#define ANKA_GRAPHICS_PRINT_OIMAGEPRINTER_HPP

#include "IImagePrinter.hpp"

#include <memory>

namespace anka::Graphics::Print
{
	/*
	*
	* Concrete Image Printer Class (GDI)
	*
	* PrintDlgEx picks the device, GDI rasterises the page, the spooler does
	* the rest; every native handle lives in an RAII wrapper so no code path
	* can leak a device context or half-open spooler job. Stateless between
	* jobs, hence a process-wide singleton like the other system-dialog
	* services (OFilePicker).
	*
	*/
	class OImagePrinter final : public IImagePrinter
	{
		public:

			// Singleton
			static OImagePrinter& getInstance();

			OImagePrinter(const OImagePrinter& printer) = delete;
			OImagePrinter(OImagePrinter&& printer) noexcept = delete;

			~OImagePrinter() noexcept override;

			OImagePrinter& operator=(const OImagePrinter& printer) = delete;
			OImagePrinter& operator=(OImagePrinter&& printer) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// IImagePrinter overrides
			//

			PrintOutcome print(PrintRequest request) const override;

		private:

			// Singleton COTOR
			OImagePrinter();

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_GRAPHICS_PRINT_OIMAGEPRINTER_HPP
