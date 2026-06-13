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

#ifndef TALAY_SETTINGS_ITALAYSETTINGS_HPP
#define TALAY_SETTINGS_ITALAYSETTINGS_HPP

#include "framework/Core/Object/IObject.hpp"
#include "Language.hpp"

#include <optional>

namespace talay
{
	namespace Settings
	{
		/*
		*
		* Abstract Talay Settings Interface
		*
		*/
		class ITalaySettings : public anka::Core::Object::IObject
		{
			public:

				virtual ~ITalaySettings() noexcept override = default;

				//
				// Load / Save
				//

				virtual bool load(void) = 0;

				virtual bool save(void) = 0;

				// Restore every setting to its factory default and persist the
				// result — not just the dialog's toggles but the window bounds,
				// language, and the conversion format + output directory too.
				// The single source of truth is the same defaults a first-run
				// ini is seeded with, so a reset and a fresh install agree.
				virtual void resetToDefaults(void) = 0;

				//
				// General
				//

				// The saved UI-language choice, or empty when none is set (then the
				// app opens in the OS display language). Stored as [General] language
				// = 0 (Turkish) / 1 (English) / empty (follow the OS). Passing
				// std::nullopt clears the choice back to "follow the OS".
				virtual std::optional<talay::Localization::Language> getLanguage(void) const = 0;
				virtual void setLanguage(std::optional<talay::Localization::Language> language) = 0;

				//
				// MainWindow
				//

				virtual int  getMainWindowPosX(void) const = 0;
				virtual void setMainWindowPosX(int value) = 0;

				virtual int  getMainWindowPosY(void) const = 0;
				virtual void setMainWindowPosY(int value) = 0;

				virtual int  getMainWindowClientWidth(void) const = 0;
				virtual void setMainWindowClientWidth(int value) = 0;

				virtual int  getMainWindowClientHeight(void) const = 0;
				virtual void setMainWindowClientHeight(int value) = 0;

				// When true, the main window stays above every other window. Off by
				// default. Stored as [MainWindow] always_on_top = 0 / 1.
				virtual bool isAlwaysOnTopEnabled(void) const = 0;
				virtual void setAlwaysOnTopEnabled(bool enabled) = 0;

				//
				// Library
				//

				// When true (the default), every successfully opened image is added
				// to the library automatically. Stored as [Library] auto_add_opened
				// = 0 / 1 in the ini; absent or unreadable means the default.
				virtual bool isAutoAddToLibraryEnabled(void) const = 0;
				virtual void setAutoAddToLibraryEnabled(bool enabled) = 0;

				//
				// Shell
				//

				// When true (the default), Talay lists itself under "Open with" for
				// the image types it handles. Persisted as [Shell] register_file_types
				// = 0 / 1 so the choice has a default and survives across runs (the
				// registry alone could not express "on by default"). Absent or
				// unreadable means the default; a future installer may register too.
				virtual bool isFileTypesRegistrationEnabled(void) const = 0;
				virtual void setFileTypesRegistrationEnabled(bool enabled) = 0;

				//
				// Conversion
				//

				// The last-used target format extension (no leading dot, e.g. L"png")
				// and output directory for the conversion window. Both are empty when
				// nothing has been saved yet, letting the window pick sensible defaults.
				// Stored as [Conversion] format = ... / output_dir = ... in the ini.
				virtual std::wstring getConversionFormat(void) const = 0;
				virtual void         setConversionFormat(const std::wstring& extension) = 0;

				virtual std::wstring getConversionOutputDirectory(void) const = 0;
				virtual void         setConversionOutputDirectory(const std::wstring& directory) = 0;
		};
	}
}

#endif // TALAY_SETTINGS_ITALAYSETTINGS_HPP
