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

#ifndef ANKA_CORE_IO_FOLDER_OFOLDERIMAGESCANNER_HPP
#define ANKA_CORE_IO_FOLDER_OFOLDERIMAGESCANNER_HPP

#include "IFolderImageScanner.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace anka::Core::IO
{
	/*
	*
	* Concrete Folder Image Scanner Class
	*
	*/
	class OFolderImageScanner final : public IFolderImageScanner
	{
		public:

			// Singleton
			static OFolderImageScanner& getInstance();

			OFolderImageScanner(const OFolderImageScanner& scanner) = delete;
			OFolderImageScanner(OFolderImageScanner&& scanner) noexcept = delete;

			~OFolderImageScanner() noexcept override;

			OFolderImageScanner& operator=(const OFolderImageScanner& scanner) = delete;
			OFolderImageScanner& operator=(OFolderImageScanner&& scanner) noexcept = delete;

			//
			// IObject overrides
			//

			const std::wstring& getName(void) const override;
			void                setName(const std::wstring& name) override;

			std::uint64_t getId(void) const override;

			//
			// IFolderImageScanner overrides
			//

			std::vector<std::wstring> scan(const std::wstring& imagePath,
			                               const std::vector<std::wstring>& extensions) const override;

		private:

			// Singleton COTOR
			OFolderImageScanner();

			struct Impl;
			std::unique_ptr<Impl> m_pImpl;
	};
}

#endif // ANKA_CORE_IO_FOLDER_OFOLDERIMAGESCANNER_HPP
