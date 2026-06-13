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

#ifndef ANKA_CORE_IO_FOLDER_IFOLDERIMAGESCANNER_HPP
#define ANKA_CORE_IO_FOLDER_IFOLDERIMAGESCANNER_HPP

#include "framework/Core/Object/IObject.hpp"

#include <string>
#include <vector>

namespace anka::Core::IO
{
	// Lists the image files that share a folder with a given image — the set the
	// folder reel renders. It is told which extensions count as images (the
	// caller's single source of truth, injected rather than hard-coded here), so
	// this class owns only directory traversal and ordering, not format policy.
	//
	// scan yields absolute paths, sorted case-insensitively by file name, with the
	// reference image included; an empty result means the folder held no matching
	// file (or could not be read) — the natural, throw-free "nothing to show".
	class IFolderImageScanner : public anka::Core::Object::IObject
	{
		public:

			virtual ~IFolderImageScanner() noexcept override = default;

			// Absolute paths of every file in imagePath's folder whose extension is
			// in 'extensions' (compared without case), sorted by name.
			virtual std::vector<std::wstring> scan(const std::wstring& imagePath,
			                                       const std::vector<std::wstring>& extensions) const = 0;
	};
}

#endif // ANKA_CORE_IO_FOLDER_IFOLDERIMAGESCANNER_HPP
