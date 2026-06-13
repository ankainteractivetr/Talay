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

#ifndef ANKA_CORE_FILE_OFILE_HPP
#define ANKA_CORE_FILE_OFILE_HPP

#include "IFile.hpp"

#include <filesystem>
#include <memory>
#include <string>

namespace anka
{
	namespace Core
	{
		namespace File
		{
			/*
			*
			* Concrete OFile Class
			*
			*/
			class OFile final : public IFile
			{
				public:


				   //
					// COTORs and DOTOR
					//

				   OFile();
					OFile(const std::wstring& objectName, 
							const std::wstring& fileName, 
							const std::filesystem::path& filePath);
					OFile(const OFile& object) = delete;
					OFile(OFile&& object) noexcept = delete;

					~OFile() noexcept override;

					//
					// Getters and Setters
					//

					const std::wstring& getFileName(void) const override;
					void                setFileName(const std::wstring& fileName) override;

					const std::filesystem::path& getFilePath(void) const override;
					void                         setFilePath(const std::filesystem::path& path) override;

					//
					// Operators
					//

					OFile& operator=(const OFile& object) = delete;
					OFile& operator=(OFile&& object) noexcept = delete;

					//
					// IObject overrides
					//

					const std::wstring& getName(void) const override;
					void                setName(const std::wstring& name) override;

					std::uint64_t getId(void) const override;

					//
					// IFile overrides
					//

					bool open(void) override;

					bool isOpen(void) const override;

				   bool close(void) override;

					bool write(const void* buffer, std::size_t size) override;

					bool truncate(void) override;

					std::vector<unsigned char> readAll(void) override;


				private:


					struct Impl;
					std::unique_ptr<Impl> m_pImpl;

			};
		}
	}
}

#endif // ANKA_CORE_FILE_OFILE_HPP