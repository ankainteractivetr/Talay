#pragma once
#include "MainWindow.g.h"
#include "ITalaySettings.hpp"
#include "OSettingsDialog.hpp"
#include "framework/Core/IO/FilePicker/OFilePicker.hpp"
#include "framework/GUI/DragDrop/IFileDropTarget.hpp"
#include "framework/GUI/Presentation/IImagePresenter.hpp"
#include "framework/GUI/Viewport/IImageViewport.hpp"
#include "framework/GUI/Cursor/IHoverCursorController.hpp"
#include "IFolderReel.hpp"
#include "ILibraryPanel.hpp"
#include "IImageLibrary.hpp"
#include "ILibraryReconciler.hpp"
#include "ILibraryTransfer.hpp"
#include "framework/Core/RecentFiles/IRecentFilesTracker.hpp"
#include "framework/GUI/Menu/IRecentFilesMenu.hpp"
#include "IPrintPresenter.hpp"
#include "ISharePresenter.hpp"
#include "ISaveAsPresenter.hpp"
#include "IEffectCatalog.hpp"
#include "IShellIntegration.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

// Forward-declared so the window depends only on the dialog abstraction; the
// concrete service is pulled in by the .cpp.
namespace anka::GUI::Dialog { class IDialogService; }

// Forward-declared: only metadata references appear in this header; the full type
// is pulled in by the .cpp that reads it.
namespace anka::Graphics::Image { class ImageMetadata; }

namespace winrt::Talay::implementation
{
    struct ConversionWindow;

    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();

        // Cross-instance entry points (not projected — used by App): open a file
        // handed over on the command line, and react to a payload another Talay
        // process forwarded over the instance channel.
        void openImageFile(std::wstring const& path);
        void onInstanceMessage(std::wstring const& payload);

        // Window controls
        void OnMinimizeClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMaximizeClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnCloseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Menu — File
        winrt::fire_and_forget OnMenuFileOpen(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuFileConvert(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        winrt::fire_and_forget OnMenuFileImportLibrary(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        winrt::fire_and_forget OnMenuFileExportLibrary(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuFileShare(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuFilePrint(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuFileExit(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Menu — Effects (every effect radio routes through OnMenuEffectPicked;
        // its Tag names the EffectKind). Save As lives here too — it writes the
        // image with the active effect baked in.
        void OnMenuEffectPicked(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuEffectSaveAs(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuEffectReset(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Menu — Settings
        void OnMenuOptionsClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuSettingsResetAll(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Menu — Help
        void OnMenuHelpGuide(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuHelpSourceCode(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnMenuHelpAbout(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Toolbar
        void OnFolderBtnClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnFavBtnClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLibAddBtnClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnInfoBtnClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnRotateBtnClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnZoomInClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnZoomOutClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnZoomSliderChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::Controls::Primitives::RangeBaseValueChangedEventArgs const&);
        void OnFullscreenBtnClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Library panel
        void OnLibHandleClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLibCloseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnLibResetFilters(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Reel panel
        void OnReelCollapseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Info modal
        void OnInfoCloseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnInfoModalBackdropClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Guide modal
        void OnGuideCloseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnGuideModalBackdropClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // About modal
        void OnAboutCloseClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnAboutModalBackdropClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);
        void OnAboutHomepageClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Fullscreen exit
        void OnFsExitClick(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::RoutedEventArgs const&);

        // Viewport clip
        void OnViewportSizeChanged(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::SizeChangedEventArgs const&);

        // Window bounds persistence
        void OnAppWindowChanged(Microsoft::UI::Windowing::AppWindow const&, Microsoft::UI::Windowing::AppWindowChangedEventArgs const&);
        void OnWindowClosed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&);

        // Conversion dialog lifetime
        void OnConversionWindowClosed(Windows::Foundation::IInspectable const&, Microsoft::UI::Xaml::WindowEventArgs const&);

    private:

        std::unique_ptr<talay::Settings::ITalaySettings> m_settings;

        // Last known "restored" window bounds, cached as the window moves/resizes
        // and persisted on close.
        int m_winX{ 100 };
        int m_winY{ 100 };
        int m_winW{ 1280 };
        int m_winH{ 720 };

        // Full path of the image currently shown, set on a successful open. Empty
        // when nothing is open — File › Convert is a no-op in that state.
        std::wstring m_currentImagePath;

        // Keeps the open conversion dialog alive for its (owned, modal) lifetime;
        // both null while none is open. The implementation pointer exists so
        // forwarded convert requests can extend the open batch (appendSources).
        Microsoft::UI::Xaml::Window m_conversionWindow{ nullptr };
        winrt::com_ptr<ConversionWindow> m_conversion{ nullptr };

        // The settings modal pair (MVP), created lazily on first open and then
        // reused so a pending ShowAsync never loses its objects. The coordinator
        // is declared first so it outlives the presenter that references it.
        std::unique_ptr<talay::Shell::IShellIntegration> m_shellIntegration;
        std::unique_ptr<talay::Settings::ISettingsPresenter> m_settingsPresenter;
        std::unique_ptr<talay::GUI::Settings::OSettingsDialog> m_settingsDialog;

        // The print glue (MVP), created lazily on first File › Print and then
        // reused — its collaborators are process-wide services.
        std::unique_ptr<talay::Print::IPrintPresenter> m_printPresenter;

        // The share glue (MVP), created lazily on first File › Share and then
        // reused — its collaborators are process-wide services.
        std::unique_ptr<talay::Share::ISharePresenter> m_sharePresenter;

        // The view effect currently on the shown image (None = original) and
        // the catalog that maps a menu choice to its framework effect, created
        // lazily on the first choice. View-only state: every newly opened
        // image starts back at None.
        talay::Effects::EffectKind m_effectKind{ talay::Effects::EffectKind::None };
        std::unique_ptr<talay::Effects::IEffectCatalog> m_effectCatalog;

        // The save-as glue (MVP), created lazily on first File › Save As and
        // then reused — its collaborator is the process-wide converter.
        std::unique_ptr<talay::Save::ISaveAsPresenter> m_saveAsPresenter;

        bool m_libOpen{ false };
        bool m_reelCollapsed{ true };
        bool m_infoOpen{ false };
        bool m_fullscreen{ false };

        // Re-entrancy guard: SyncZoomControls writes the slider, whose
        // ValueChanged would otherwise drive the viewport again in a loop.
        bool m_syncingZoom{ false };

        // Presents the opened image (still or animation) into RenderImage: it owns
        // the Direct2D surface, the WriteableBitmap it draws into, and the playback
        // timer, so the window no longer juggles any of that itself.
        std::unique_ptr<anka::GUI::Presentation::IImagePresenter> m_presenter;

        // Owns the on-screen zoom / rotation / pan transform of the shown image.
        std::unique_ptr<anka::GUI::Viewport::IImageViewport> m_viewport;

        // Drives the window's hover cursor (hand over clickables, move over a
        // pannable image). Declared after the viewport it queries, so it
        // destroys first and no late pointer event reaches a dead viewport.
        std::unique_ptr<anka::GUI::Cursor::IHoverCursorController> m_cursorController;

        // Makes the viewport a shell drop target: a supported image dragged
        // from the file explorer opens as if it came through File › Open.
        std::unique_ptr<anka::GUI::DragDrop::IFileDropTarget> m_dropTarget;

        // Owns the folder reel: builds + highlights the thumbnail strip for the open
        // image's folder, and reports thumbnail clicks back so the window opens them.
        std::unique_ptr<talay::GUI::Reel::IFolderReel> m_reel;

        // The image library (SQLite-backed repository), the panel controller
        // that filters + renders it, and the dump transfer that imports /
        // exports it. Declared in this order on purpose: members destroy in
        // reverse, so the library's dependents let go of it first.
        std::unique_ptr<talay::Library::IImageLibrary> m_library;
        std::unique_ptr<talay::GUI::Library::ILibraryPanel> m_libraryPanel;
        std::unique_ptr<talay::Library::Transfer::ILibraryTransfer> m_libraryTransfer;

        // Prunes entries whose backing file has vanished. Declared after the
        // library so it destroys first — its background sweep is joined before
        // the library it reconciles is gone.
        std::unique_ptr<talay::Library::ILibraryReconciler> m_libraryReconciler;

        // The recently-opened list (SQLite-backed tracker, same database file
        // as the library) and the menu presenter mirroring it into File ›
        // Recent Files. Declared in this order on purpose: members destroy in
        // reverse, so the menu lets go of the tracker first.
        std::unique_ptr<anka::Core::RecentFiles::IRecentFilesTracker> m_recentFiles;
        std::unique_ptr<anka::GUI::Menu::IRecentFilesMenu> m_recentFilesMenu;

        static constexpr std::size_t k_recentFilesCapacity{ 6 };

        static constexpr double k_libPanelWidth{ 880.0 };
        static constexpr double k_reelExpandedWidth{ 214.0 };
        static constexpr double k_reelCollapsedWidth{ 42.0 };
        static constexpr double k_zoomMin{ 10.0 };
        static constexpr double k_zoomMax{ 400.0 };

        // Construction helpers (keep the ctor a thin orchestrator)
        void ConfigureTitleBar();
        void RestoreWindowBounds();
        void WireWindowEvents();
        void InitializeControlState();

        // Window-bounds persistence helpers
        bool IsRestoredOverlapped(Microsoft::UI::Windowing::AppWindowPresenter const& presenter) const;
        void CacheWindowBounds(Microsoft::UI::Windowing::AppWindow const& sender);

        void AnimateLibrary(bool open);
        void AnimateReel(bool collapse);

        // Viewport interaction: create + attach the transform controller, and
        // push a zoom change back onto the slider / readout.
        void InitializeViewport();
        void InitializeCursors();
        winrt::Talay::OCursorHost WrapContentInCursorHost();
        void SyncZoomControls(double percent);

        // Viewport drop: create + attach the drop target that opens supported
        // image files dragged onto the viewport.
        void InitializeDropTarget();

        // Fullscreen helpers
        void ApplyFullscreen(bool enter);
        void SetFullscreenPresenter(bool enter);
        void ToggleChromeForFullscreen(bool enter);
        void ApplyFullscreenFraming(bool enter);

        // Animation helpers
        void AnimateTo(Microsoft::UI::Xaml::DependencyObject const& target,
                       hstring const& property, double to, int durationMs, bool dependent);
        Microsoft::UI::Xaml::Media::Animation::DoubleAnimation MakePulseAnimation();
        void StartEmblemPulse();

        Windows::Foundation::IAsyncOperation<hstring> pickImageFile(HWND hwnd);

        // Image presentation: bring up the renderer + presenter, then decode a file
        // and hand it to the presenter, framing the viewport for the new image.
        void InitializeRenderer();
        void DisplayImage(hstring const& path);
        void ReportImageOpenFailure();
        void TrackOpenedImage(std::wstring const& path);
        void RevealViewport();
        void ConcealViewport();
        void FrameOpenedImage();

        // Folder reel: create the collaborator that fills the thumbnail strip for the
        // open image's folder, and reveal the (possibly collapsed) panel when it
        // rebuilds for a new folder.
        void InitializeReel();
        talay::GUI::Reel::ReelControls CollectReelControls();
        void EnsureReelExpanded();
        void SetReelCollapsed(bool collapse);
        void RevealReelSelection();

        // Image library: stand up the repository over its database file, bind the
        // panel controller to the library XAML controls, and route toolbar toggles
        // (favorite / add-to-library) through the repository.
        void InitializeLibrary();
        void PruneMissingLibraryEntries();
        talay::GUI::Library::LibraryPanelControls CollectLibraryControls();
        void CollectTextFilterControls(talay::GUI::Library::LibraryPanelControls& controls);
        void CollectRangeFilterControls(talay::GUI::Library::LibraryPanelControls& controls);
        void CollectFlagFilterControls(talay::GUI::Library::LibraryPanelControls& controls);
        void CollectResultControls(talay::GUI::Library::LibraryPanelControls& controls);
        void CollectPagerControls(talay::GUI::Library::LibraryPanelControls& controls);
        void AddCurrentToLibrary();
        void RemoveCurrentFromLibrary();
        void OpenFromLibrary(std::wstring const& path);
        void ConvertFromLibrary(std::wstring const& path);
        void CollapseLibrary();
        void RemoveFromLibrary(std::wstring const& path);
        void UnfavoriteInLibrary(std::wstring const& path);
        void ReportLibraryUpdateFailure();
        void AutoAddToLibrary();
        void SyncLibraryToggles();

        // Recent files: stand up the tracker over the library's database file
        // (pruning entries whose file is gone), bind the menu presenter to the
        // File › Recent Files submenu, and open a picked entry — unless it
        // vanished since, in which case it leaves the list with an apology.
        void InitializeRecentFiles();
        void OpenRecentFile(std::wstring const& path);
        void ReportRecentFileMissing();

        // Library dump transfer: pickers for the .sql dump file and the
        // summary dialogs the menu handlers report back through.
        Windows::Foundation::IAsyncOperation<hstring> pickDumpFileToOpen();
        Windows::Foundation::IAsyncOperation<hstring> pickDumpFileToSave();
        void ShowImportResult(talay::Library::Transfer::TransferSummary const& summary);
        void ShowExportResult(bool succeeded);

        // Print: build the presenter once, then report how the job ended —
        // quietly for success and cancel, with a dialog only on failure.
        void EnsurePrintPresenter();
        void ShowPrintResult(anka::Graphics::Print::PrintOutcome outcome);

        // Share: build the presenter once, then report how the copy ended —
        // a confirmation on success (a clipboard copy has no visible effect
        // of its own), the error on failure. With a view effect active, the
        // user first picks which version to publish.
        void EnsureSharePresenter();
        void ReportShareWithoutImage();
        void AskShareChoice();
        void ShareImage(bool withEffect);
        void ShowShareResult(bool succeeded);

        // Effects: route a menu choice into the presenter (rejecting it
        // without an image), mirror the active choice on the radio items, and
        // fall back to None whenever a new image arrives effect-free.
        void ApplyEffect(talay::Effects::EffectKind kind);
        void EnsureEffectCatalog();
        void SyncEffectMenu();
        void RejectEffectWithoutImage();
        void ResetEffectSelection();
        std::shared_ptr<const anka::Graphics::Effect::IPixelEffect> currentEffect() const;

        // Save As (Effects menu): one dialog picks directory, name AND format
        // (a type per writable format) and the file is written with the active
        // effect baked in — no prompt, what you see is what you save.
        void EnsureSaveAsPresenter();
        void ReportSaveAsWithoutImage();
        winrt::fire_and_forget SaveImageAs();
        Windows::Foundation::IAsyncOperation<hstring> pickSaveDestination();
        std::vector<anka::Core::IO::SaveFileType> CollectSaveFileTypes();
        void RememberSaveFormat(std::wstring const& destinationPath);
        void ShowSaveAsResult(bool succeeded);

        // Open the modal conversion dialog over this window, or extend the one
        // already open with more files (forwarded convert requests).
        void OpenConversionWindowFor(std::vector<std::wstring> const& files);
        void RouteConvertRequest(std::vector<std::wstring> const& files);
        void BringToFront();
        void RestoreIfMinimized();

        // Settings modal: build the MVP pair once, react to a confirmed apply
        // (always-on-top lands live, a language change offers a restart) and
        // run the guarded reset-library flow.
        void EnsureSettingsPresenter();
        void OnSettingsApplied(talay::Settings::SettingsApplyResult const& result);
        void ApplyAlwaysOnTop(bool enabled);
        void PromptRestart();
        void Relaunch();
        void ConfirmLibraryReset();
        void ResetLibrary();
        void ConfirmResetAllSettings();
        void ResetAllSettings();
        void AnnounceSettingsReset(bool languageChanged);

        // Exit confirmation shared by the title-bar X and File › Exit.
        void ConfirmExit();

        // Help links: hand a project web address to the default browser, turning
        // a refused launch into an error dialog instead of a silent no-op.
        void OpenWebLink(std::wstring const& url);

        // File-info presentation: read the open image's metadata once, then fan it
        // out to the info modal rows, the toolbar chips and the viewport HUD.
        void RejectInfoWithoutImage();
        void PopulateImageInfo(hstring const& path);
        void UpdateInfoModal(anka::Graphics::Image::ImageMetadata const& meta);
        void UpdateInfoIdentity(anka::Graphics::Image::ImageMetadata const& meta);
        void UpdateInfoGeometry(anka::Graphics::Image::ImageMetadata const& meta);
        void UpdateInfoFileFacts(anka::Graphics::Image::ImageMetadata const& meta);
        void UpdateInfoChips(anka::Graphics::Image::ImageMetadata const& meta);
        void UpdateInfoHud(anka::Graphics::Image::ImageMetadata const& meta);

        // Themed dialog service, with this window's XamlRoot refreshed as host so
        // every dialog anchors onto the live content.
        anka::GUI::Dialog::IDialogService& dialogs();

        Microsoft::UI::Windowing::OverlappedPresenter GetPresenter();

        HWND getHWND() const;
        HWND EnsureWindowHandle();

    };
}

namespace winrt::Talay::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}
