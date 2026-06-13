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

#ifndef ANKA_CORE_FILE_IFILE_HPP
#define ANKA_CORE_FILE_IFILE_HPP

#include <string>
#include <filesystem>
#include <vector>

#include "framework/Core/Object/IObject.hpp"

namespace anka
{
	namespace Core
	{
		namespace File
		{
			/*
			* Abstract File Interface
			*
			* A File is the mother class of all IO File entities
			*
			*/
			
			class IFile : public anka::Core::Object::IObject
			{
				public:


					//
					// COTORs and DOTOR
					//

					virtual ~IFile() noexcept = default;

					//
					// Getters and Setters
					//

					virtual const std::wstring& getFileName(void) const = 0;
					virtual void                setFileName(const std::wstring& fileName) = 0;

					virtual const std::filesystem::path& getFilePath(void) const = 0;
					virtual void                         setFilePath(const std::filesystem::path& path) = 0;

					//
					// Other member functions
					//

					virtual bool open(void) = 0;

					virtual bool isOpen(void) const = 0;

					virtual bool close(void) = 0;

					virtual bool write(const void* buffer, std::size_t size) = 0;

					virtual bool truncate(void) = 0;

					virtual std::vector<unsigned char> readAll(void) = 0;
			};
		}
	}
}

#endif // ANKA_CORE_FILE_IFILE_HPP