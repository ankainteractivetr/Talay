#pragma once
#include "ConversionWindow.g.h"
#include "ITalaySettings.hpp"
#include "framework/System/Windows/windows.hpp"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// Forward-declared so the window depends only on the dialog abstraction; the
// concrete service is pulled in by the .cpp.
namespace anka::GUI::Dialog { class IDialogService; }

namespace winrt::Talay::implementation
{
    // A modal-style conversion dialog hosted in its own WinUI window. It is opened
    // by MainWindow when an image is loaded (centred over it, with the main window
    // disabled until this closes), but is self-sufficient: given no owner it
    // centres on the desktop, so the app can also start straight into it.
    //
    // The chosen format and output directory round-trip through the .ini via
    // ITalaySettings; the actual pixel work is delegated to OImageConverter, so this
    // class owns only presentation and the modal lifetime.
    struct ConversionWindow : ConversionWindowT<ConversionWindow>
    {
        ConversionWindow();

        // Bind the window to its source images and (optional) owner, fill the format
        // list with the writable subset of 'supportedFormats', load the saved
        // preferences and centre it. Called once after construction, before
        // Activate(). Not projected — it is only used from C++ (MainWindow / App).
        void configure(std::vector<std::wstring> const& sourcePaths, HWND owner,
                       std::vector<std::wstring> const& supportedFormats);

        // Merge more source files into an already-open dialog (no duplicates) and
        // bring it to the front — how files from later "Convert with Talay"
        // invocations join the batch over the instance channel.
        void appendSources(std::vector<std::wstring> const& sourcePaths);

        // Title bar
        void OnCloseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Actions
        void OnBrowseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnCancelClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnConvertClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Modal lifetime
        void OnWindowClosed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&);

    private:

        std::unique_ptr<talay::Settings::ITalaySettings> m_settings;
        std::vector<std::wstring>                        m_sourcePaths;
        std::vector<std::wstring>                        m_supportedFormats;
        HWND                                             m_owner {nullptr};

        static constexpr int k_windowWidth  {460};
        static constexpr int k_windowHeight {500};

        // Construction / configuration helpers (keep each method a thin step).
        void ConfigureTitleBar();
        void ConfigurePresenter();
        void BuildFormatList();
        void LoadPreferences();
        void PrefillFromSource();
        void CenterOnOwner();
        void DisableOwner(bool disable);
        void selectFormat(std::wstring const& extension);
        std::wstring sourceDirectory() const;

        // Source presentation: one path reads as itself, a batch as a count
        // with the full list in the tooltip; the name box only applies to a
        // single file (a batch keeps each file's own stem).
        bool isBatch() const;
        void RefreshSourceSummary();
        void SyncNameBoxMode();
        std::wstring sourceSummary() const;
        std::wstring joinedSources() const;

        // Convert-click pipeline: read the form, persist the choices, run the
        // conversion over every source and report the outcome.
        talay::Settings::ITalaySettings& settings();
        std::wstring SelectedExtension();
        std::wstring ResolvedOutputDirectory();
        bool IsFormValid(std::wstring const& directory);
        std::wstring OutputNameFor(std::wstring const& source);
        std::size_t ConvertAll(std::wstring const& extension, std::wstring const& directory);
        void PersistChoices(std::wstring const& extension, std::wstring const& directory);
        void ReportOutcome(std::size_t converted, std::size_t total);
        std::wstring doneMessage(std::size_t total) const;
        std::wstring partialMessage(std::size_t converted, std::size_t total) const;

        // Themed dialog service, anchored on this window's live XamlRoot.
        anka::GUI::Dialog::IDialogService& dialogs();

        HWND getHWND() const;
        Microsoft::UI::Windowing::OverlappedPresenter GetPresenter();
    };
}

namespace winrt::Talay::factory_implementation
{
    struct ConversionWindow : ConversionWindowT<ConversionWindow, implementation::ConversionWindow>
    {
    };
}
