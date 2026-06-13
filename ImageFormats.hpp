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

#ifndef TALAY_FORMATS_IMAGEFORMATS_HPP
#define TALAY_FORMATS_IMAGEFORMATS_HPP

#include <string>
#include <vector>

namespace talay::Formats
{
   // Every file extension Talay's decoder stack can open, dotted and uppercase.
   // The single source of truth shared by the viewer, the folder reel, the
   // conversion window and Windows shell registration -- when a format is added
   // here, every consumer learns about it at once.
   //
   // A function-local static (Meyers singleton) instead of a global, so the
   // list is built on first use and never races static initialization order.
   inline const std::vector<std::wstring>& supportedImageFormats(void)
   {
      static const std::vector<std::wstring> s_formats {
         L".BMP",L".CUT",L".DDS",L".EXR",L".FAXG3",L".GIF",L".HDR",
         L".ICO",L".IFF",L".LBM",L".J2K",L".JNG",L".JP2",L".JPG",
         L".JPEG",L".JXR",L".WDP",L".HDP",L".KOA",L".MNG",L".PBM",
         L".PCD",L".PCX",L".PFM",L".PGM",L".PICT",L".PCT",L".PNG",
         L".PPM",L".PSD",L".RAS",L".SGI",L".TGA",L".TARGA",L".TIF",
         L".TIFF",L".WBMP",L".WEBP",L".XBM",L".XPM",L".RAW",L".3FR",
         L".ARW",L".CR2",L".CRW",L".DNG",L".ERF",L".KDC",L".MEF",
         L".MOS",L".MRW",L".NEF",L".NRW",L".ORF",L".PEF",L".RAF",
         L".RW2",L".SR2",L".SRF"
      };

      return s_formats;
   }
}

#endif // TALAY_FORMATS_IMAGEFORMATS_HPP
