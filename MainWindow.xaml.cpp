#include "pch.h"
#include "MainWindow.xaml.h"
#include "ConversionWindow.xaml.h"
#include "ImageFormats.hpp"
#include "LaunchRequest.hpp"
#include "OEffectCatalog.hpp"
#include "OPrintPresenter.hpp"
#include "OSaveAsPresenter.hpp"
#include "OSettingsPresenter.hpp"
#include "OShellIntegration.hpp"
#include "OSharePresenter.hpp"
#include "OTalaySettings.hpp"
#include "TalayLinks.hpp"
#include "LocalizedDialog.hpp"
#include "framework/Graphics/Conversion/OImageConverter.hpp"
#include "framework/Graphics/Print/OImagePrinter.hpp"
#include "framework/System/Clipboard/OImageClipboard.hpp"
#include "framework/System/Shell/OLinkOpener.hpp"
#include "framework/System/Shell/OShellRegistrar.hpp"
#include "framework/System/Time/UnixTime.hpp"
#include "framework/System/Process/ExecutablePath.hpp"

#include <shellapi.h>
#include "framework/Core/IO/FilePicker/OFilePicker.hpp"
#include "framework/Core/Error/Exception/OException.hpp"
#include "framework/Core/Error/ExceptionHandler/OExceptionHandler.hpp"
#include "framework/GUI/Dialog/ODialogService.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"
#include "framework/Graphics/Renderer/ORenderer.hpp"
#include "framework/Graphics/Image/OImageFileReader.hpp"
#include "framework/Graphics/Image/OImageMetadataReader.hpp"
#include "framework/Graphics/Image/ImageInfoText.hpp"
#include "framework/GUI/Viewport/OImageViewport.hpp"
#include "framework/GUI/DragDrop/OFileDropTarget.hpp"
#include "framework/GUI/Presentation/OImagePresenter.hpp"
#include "framework/GUI/Cursor/OHoverCursorController.hpp"
#include "framework/GUI/Cursor/OClickableCursorRule.hpp"
#include "framework/GUI/Cursor/OViewportDragCursorRule.hpp"
#include "OCursorHost.h"
#include "OFolderReel.hpp"
#include "OLibraryPanel.hpp"
#include "OImageLibrary.hpp"
#include "OSqliteLibraryStore.hpp"
#include "OLibraryReconciler.hpp"
#include "OLibraryTransfer.hpp"
#include "framework/Core/Database/OSqliteDatabase.hpp"
#include "framework/Core/RecentFiles/ORecentFilesTracker.hpp"
#include "framework/Core/RecentFiles/OSqliteRecentFilesStore.hpp"
#include "framework/GUI/Menu/ORecentFilesMenu.hpp"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

namespace winrt
{
   using namespace Microsoft::UI::Xaml;
   using namespace Microsoft::UI::Xaml::Controls;
   using namespace Microsoft::UI::Xaml::Controls::Primitives;
   using namespace Microsoft::UI::Xaml::Input;
   using namespace Microsoft::UI::Xaml::Media;
   using namespace Microsoft::UI::Xaml::Media::Animation;
   using namespace Microsoft::UI::Windowing;
   using namespace Windows::Foundation;
}

using namespace anka::GUI::Dialog;

using talay::GUI::showLocalizedMessage;

using anka::Core::Localization::OLocalizationService;
using anka::Graphics::Renderer::ORenderer;
using anka::Graphics::Image::OImageFileReader;
using anka::GUI::Presentation::OImagePresenter;
using anka::Graphics::Image::OImageMetadataReader;
using anka::Graphics::Image::ImageMetadata;
using anka::Graphics::Image::ImageSequence;
using anka::GUI::Cursor::CursorShape;
using anka::GUI::Cursor::ICursorRule;
using anka::GUI::Cursor::OClickableCursorRule;
using anka::GUI::Cursor::OHoverCursorController;
using anka::GUI::Cursor::OViewportDragCursorRule;
using talay::GUI::Reel::OFolderReel;
using talay::GUI::Reel::ReelControls;
using talay::GUI::Library::LibraryPanelControls;
using talay::GUI::Library::OLibraryPanel;
using talay::Library::LibraryEntry;
using talay::Library::OImageLibrary;
using talay::Library::OLibraryReconciler;
using talay::Library::Store::OSqliteLibraryStore;
using talay::Library::Transfer::OLibraryTransfer;
using talay::Library::Transfer::TransferSummary;
using anka::Core::Database::OSqliteDatabase;
using anka::Core::RecentFiles::ORecentFilesTracker;
using anka::Core::RecentFiles::OSqliteRecentFilesStore;
using anka::GUI::Menu::ORecentFilesMenu;
using anka::Graphics::Print::OImagePrinter;
using anka::Graphics::Print::PrintOutcome;
using anka::System::Clipboard::OImageClipboard;
using anka::System::Shell::OLinkOpener;
using talay::Print::OPrintPresenter;
using talay::Share::OSharePresenter;
using talay::Save::OSaveAsPresenter;
using talay::Effects::EffectKind;
using talay::Effects::OEffectCatalog;
using anka::Core::IO::SaveFileType;
using anka::Graphics::Conversion::OImageConverter;
using anka::Graphics::Conversion::WritableFormat;
using anka::Graphics::Effect::IPixelEffect;

namespace InfoText = anka::Graphics::Image::ImageInfoText;

namespace
{
   // Helper: milliseconds → WinUI Duration
   winrt::Microsoft::UI::Xaml::Duration MS(int64_t ms)
   {
      return winrt::Microsoft::UI::Xaml::Duration {
         winrt::Windows::Foundation::TimeSpan {ms * 10'000LL}
      };
   }

   winrt::Microsoft::UI::Xaml::Media::Animation::CubicEase EaseOut()
   {
      auto e = winrt::Microsoft::UI::Xaml::Media::Animation::CubicEase {};
      e.EasingMode(winrt::Microsoft::UI::Xaml::Media::Animation::EasingMode::EaseOut);
      return e;
   }

   void fillEntryIdentity(LibraryEntry& entry, const ImageMetadata& meta)
   {
      entry.filePath = meta.filePath;
      entry.fileName = meta.fileName;
      entry.folder   = std::filesystem::path {meta.filePath}.parent_path().wstring();
      entry.format   = meta.format;
   }

   void fillEntryFacts(LibraryEntry& entry, const ImageMetadata& meta)
   {
      entry.fileSizeBytes = meta.fileSizeBytes;
      entry.width         = meta.width;
      entry.height        = meta.height;
      entry.frameCount    = meta.frameCount;
      entry.modifiedUtc   = meta.modifiedUnixUtc;
   }

   // Everything the library row needs, read once from the file on disk.
   LibraryEntry buildLibraryEntry(const std::wstring& path)
   {
      const ImageMetadata meta = anka::Graphics::Image::OImageMetadataReader::getInstance().read(path);

      LibraryEntry entry;
      fillEntryIdentity(entry, meta);
      fillEntryFacts(entry, meta);
      entry.addedUtc = anka::System::Time::currentUnixSeconds();
      return entry;
   }

   // Float the preferred format (PNG out of the box, via the saved conversion
   // default) to the front so the save dialog opens with it selected —
   // FileSavePicker preselects the first FileTypeChoices entry.
   void preferFormat(std::vector<SaveFileType>& types, const std::wstring& extension)
   {
      const std::wstring dotted {L"." + extension};
      const auto found {std::ranges::find_if(types,
         [&dotted](const SaveFileType& type) { return type.extension == dotted; })};

      if (found != types.end())
         std::rotate(types.begin(), found, found + 1);
   }
}

namespace winrt::Talay::implementation
{
   MainWindow::MainWindow()
   {
      InitializeComponent();
      ConfigureTitleBar();
      RestoreWindowBounds();
      ApplyAlwaysOnTop(m_settings->isAlwaysOnTopEnabled());
      WireWindowEvents();
      InitializeControlState();
      InitializeRenderer();
      InitializeViewport();
      InitializeCursors();
      InitializeDropTarget();
      InitializeReel();
      InitializeLibrary();
      InitializeRecentFiles();
   }

   // Remove DWM native caption buttons while keeping the resize border.
   // ExtendsContentIntoTitleBar alone only extends content — DWM still paints its
   // own min/max/close overlay on top; SetBorderAndTitleBar removes the entire
   // non-client title bar frame so only our XAML buttons exist. Extending is still
   // needed so SetTitleBar can register the drag region.
   void MainWindow::ConfigureTitleBar()
   {
      GetPresenter().SetBorderAndTitleBar(true,false);
      ExtendsContentIntoTitleBar(true);
      SetTitleBar(TitleBarDragArea());
   }

   void MainWindow::RestoreWindowBounds()
   {
      m_settings = std::make_unique<talay::Settings::OTalaySettings>(L"OTalaySettings",anka::System::Process::executableDirectory());
      m_settings->load();

      m_winX = m_settings->getMainWindowPosX();
      m_winY = m_settings->getMainWindowPosY();
      m_winW = m_settings->getMainWindowClientWidth();
      m_winH = m_settings->getMainWindowClientHeight();

      AppWindow().MoveAndResize({m_winX,m_winY,m_winW,m_winH});
   }

   // AppWindow().Closing does NOT fire for a programmatic Window.Close(), which is
   // the only way this app closes (custom title bar, no system close button). So we
   // cache the window bounds as they change and persist them in Window.Closed,
   // which fires for every close path (custom button, Exit menu, Alt+F4).
   void MainWindow::WireWindowEvents()
   {
      AppWindow().Changed({this,&MainWindow::OnAppWindowChanged});
      Closed({this,&MainWindow::OnWindowClosed});
   }

   void MainWindow::InitializeControlState()
   {
      FolderBtn().IsChecked(false);   // reel collapsed = unchecked
      FavBtn().IsChecked(false);      // library starts closed
      LibAddBtn().IsChecked(false);

      StartEmblemPulse();
      SyncZoomControls(100.0);   // native scale until an image is opened
   }

   // ── Window controls ──────────────────────────────────────────────────────

   void MainWindow::OnMinimizeClick(IInspectable const&,RoutedEventArgs const&)
   {
      GetPresenter().Minimize();
   }

   void MainWindow::OnMaximizeClick(IInspectable const&,RoutedEventArgs const&)
   {
      auto p = GetPresenter();
      if (p.State() == OverlappedPresenterState::Maximized)
         p.Restore();
      else
         p.Maximize();
   }

   void MainWindow::OnCloseClick(IInspectable const&,RoutedEventArgs const&)
   {
      ConfirmExit();
   }

   void MainWindow::OnAppWindowChanged(Microsoft::UI::Windowing::AppWindow const& sender,
                                       Microsoft::UI::Windowing::AppWindowChangedEventArgs const& args)
   {
      if (!args.DidPositionChange() && !args.DidSizeChange())
         return;

      if (IsRestoredOverlapped(sender.Presenter()))
         CacheWindowBounds(sender);
   }

   // True only for a normal (non-fullscreen, non-minimized) overlapped window.
   // In fullscreen the presenter is not Overlapped (casting would throw) and its
   // size is the whole monitor; a minimized window reports off-screen (-32000)
   // coordinates. Both are skipped so we keep the last good restored bounds.
   bool MainWindow::IsRestoredOverlapped(Microsoft::UI::Windowing::AppWindowPresenter const& presenter) const
   {
      if (presenter.Kind() != AppWindowPresenterKind::Overlapped)
         return false;

      return presenter.as<OverlappedPresenter>().State() != OverlappedPresenterState::Minimized;
   }

   void MainWindow::CacheWindowBounds(Microsoft::UI::Windowing::AppWindow const& sender)
   {
      const auto pos = sender.Position();
      const auto size = sender.Size();

      m_winX = pos.X;
      m_winY = pos.Y;
      m_winW = size.Width;
      m_winH = size.Height;
   }

   void MainWindow::OnWindowClosed(IInspectable const&,WindowEventArgs const&)
   {
      m_presenter->clear();
      ORenderer::getInstance().deinitialize();

      // Re-read the file first: the conversion dialog may have written [Conversion]
      // keys after we loaded at startup, and save() rewrites the whole file from
      // memory — reloading merges those keys back in so they are not lost here.
      m_settings->load();

      m_settings->setMainWindowPosX(m_winX);
      m_settings->setMainWindowPosY(m_winY);
      m_settings->setMainWindowClientWidth(m_winW);
      m_settings->setMainWindowClientHeight(m_winH);
      m_settings->save();
   }

   // ── Menu — File ──────────────────────────────────────────────────────────
   winrt::fire_and_forget MainWindow::OnMenuFileOpen(IInspectable const&,RoutedEventArgs const&)
   {
      try {
         auto lifetime {get_strong()};   // keep the window alive across the co_await

         HWND hwnd {EnsureWindowHandle()};

         const hstring path {co_await pickImageFile(hwnd)};

         //Hop back to the UI thread — pickFile can resume us on a background thread.
         co_await wil::resume_foreground(DispatcherQueue());

         if (path.empty()) co_return;

         DisplayImage(path);

      } catch (const std::exception& exception) {
         anka::Core::Error::ExceptionHandler::OExceptionHandler::getInstance().handleException(exception);
      } catch (const std::shared_ptr<anka::Core::Error::Exception::OException>& exception) {
         anka::Core::Error::ExceptionHandler::OExceptionHandler::getInstance().handleException(exception);
      }
   }

   // Shows the system file picker for supported image formats and returns the
   // chosen path (empty if the user cancels). Coroutine: pickFile may resume on
   // a background thread, so the caller is responsible for hopping back to the
   // UI thread before touching XAML.
   winrt::Windows::Foundation::IAsyncOperation<hstring> MainWindow::pickImageFile(HWND hwnd)
   {
      auto& picker {anka::Core::IO::OFilePicker::getInstance()};
      picker.setOwner(hwnd);
      hstring path {co_await picker.pickFile(talay::Formats::supportedImageFormats())};
      co_return path;
   }

   // Convert is meaningful only when an image is on screen; otherwise tell the user
   // to open one first rather than launching an empty dialog.
   void MainWindow::OnMenuFileConvert(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         showLocalizedMessage(dialogs(), L"ConvertNoImageTitle", L"ConvertNoImageMessage", DialogSeverity::Information);
         return;
      }

      OpenConversionWindowFor({m_currentImagePath});
   }

   // Build the dialog, point it at the given images with this window as its owner,
   // keep it alive for its modal lifetime and show it.
   void MainWindow::OpenConversionWindowFor(std::vector<std::wstring> const& files)
   {
      // The dialog is modal over this window (it disables and centres on it). A
      // forwarded "Convert" can arrive while we are minimized, so restore first —
      // otherwise it centres on the off-screen minimized rect and the disabled
      // owner is left unreachable, freezing the app on restore.
      RestoreIfMinimized();

      auto conversion {winrt::make_self<implementation::ConversionWindow>()};
      conversion->configure(files, getHWND(), talay::Formats::supportedImageFormats());
      conversion->Closed({get_weak(), &MainWindow::OnConversionWindowClosed});

      m_conversion = conversion;
      m_conversionWindow = conversion.as<Window>();
      m_conversionWindow.Activate();
   }

   void MainWindow::OnConversionWindowClosed(IInspectable const&, WindowEventArgs const&)
   {
      m_conversion = nullptr;
      m_conversionWindow = nullptr;
   }

   // ── Cross-instance entry points ──────────────────────────────────────────

   void MainWindow::openImageFile(std::wstring const& path)
   {
      DisplayImage(hstring {path});
   }

   // A later Talay process forwarded its launch over the instance channel:
   // convert requests open (or extend) the conversion dialog; everything else
   // is an "open" — come to the front and show the file when one came along.
   void MainWindow::onInstanceMessage(std::wstring const& payload)
   {
      const talay::Launch::LaunchRequest request {talay::Launch::decodePayload(payload)};

      if (request.mode == talay::Launch::LaunchMode::Convert && !request.files.empty()) {
         RouteConvertRequest(request.files);
         return;
      }

      BringToFront();
      if (!request.files.empty())
         DisplayImage(hstring {request.files.front()});
   }

   // Pull the window back to the user: restore a minimized frame first, then
   // take the foreground — the forwarding process granted us the right before
   // it sent the message.
   void MainWindow::BringToFront()
   {
      RestoreIfMinimized();
      SetForegroundWindow(getHWND());
   }

   // Un-minimize the frame so it is back on screen; a window in any other state
   // is left untouched. Restoring is synchronous, so a caller may then read the
   // window's real rect or open an owned modal centred on it.
   void MainWindow::RestoreIfMinimized()
   {
      const bool overlapped {AppWindow().Presenter().Kind() == AppWindowPresenterKind::Overlapped};
      if (overlapped && GetPresenter().State() == OverlappedPresenterState::Minimized)
         GetPresenter().Restore();
   }

   // An already-open dialog absorbs the new files; otherwise one opens for them.
   void MainWindow::RouteConvertRequest(std::vector<std::wstring> const& files)
   {
      if (m_conversion) {
         m_conversion->appendSources(files);
         return;
      }

      OpenConversionWindowFor(files);
   }

   // ── Menu — File › Save As ────────────────────────────────────────────────

   // Save As (in the Effects menu) re-encodes the open image wherever, under
   // whatever name, and in whatever format the picker chooses — always with the
   // effect currently on screen baked in, so what you see is what you save.
   void MainWindow::OnMenuEffectSaveAs(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         ReportSaveAsWithoutImage();
         return;
      }

      EnsureSaveAsPresenter();
      SaveImageAs();
   }

   void MainWindow::ReportSaveAsWithoutImage()
   {
      showLocalizedMessage(dialogs(), L"SaveAsNoImageTitle", L"SaveAsNoImageMessage", DialogSeverity::Information);
   }

   // Lazy MVP wiring: the collaborator is a process-wide service, so one
   // presenter binds it once and serves every save after that.
   void MainWindow::EnsureSaveAsPresenter()
   {
      if (!m_saveAsPresenter)
         m_saveAsPresenter = std::make_unique<OSaveAsPresenter>(OImageConverter::getInstance());
   }

   // The active view effect (none → null) baked into the saved file. The source
   // and effect are captured before the picker suspends, since a forwarded open
   // could change either while the dialog is up.
   winrt::fire_and_forget MainWindow::SaveImageAs()
   {
      try {
         auto lifetime {get_strong()};   // keep the window alive across the co_await

         const std::wstring source {m_currentImagePath};
         const std::shared_ptr<const IPixelEffect> effect {currentEffect()};

         const hstring destination {co_await pickSaveDestination()};

         // Hop back to the UI thread — the picker can resume us elsewhere.
         co_await wil::resume_foreground(DispatcherQueue());
         if (destination.empty()) co_return;

         RememberSaveFormat(std::wstring {destination});
         ShowSaveAsResult(effect
            ? m_saveAsPresenter->saveAsWithEffect(source, std::wstring {destination}, *effect)
            : m_saveAsPresenter->saveAs(source, std::wstring {destination}));

      } catch (const std::exception& exception) {
         anka::Core::Error::ExceptionHandler::OExceptionHandler::getInstance().handleException(exception);
      }
   }

   // The effect on screen, or null when none is applied — the catalog only
   // exists once an effect has been chosen, so a missing catalog means none.
   std::shared_ptr<const anka::Graphics::Effect::IPixelEffect> MainWindow::currentEffect() const
   {
      if (!m_effectCatalog)
         return nullptr;

      return m_effectCatalog->effectFor(m_effectKind);
   }

   // The save dialog itself picks directory, name AND format: one file type
   // per writable format, defaulting the name to the open image's.
   winrt::Windows::Foundation::IAsyncOperation<hstring> MainWindow::pickSaveDestination()
   {
      auto& picker {anka::Core::IO::OFilePicker::getInstance()};
      picker.setOwner(EnsureWindowHandle());

      co_return co_await picker.pickSaveFileAs(CollectSaveFileTypes(),
         std::filesystem::path {m_currentImagePath}.stem().wstring());
   }

   std::vector<SaveFileType> MainWindow::CollectSaveFileTypes()
   {
      std::vector<SaveFileType> types;
      for (const WritableFormat& format : m_saveAsPresenter->formats())
         types.push_back({format.label, L"." + format.extension});

      preferFormat(types, m_settings->getConversionFormat());
      return types;
   }

   // Persist the format the user just chose so the next Save As (and Convert)
   // opens with it selected; PNG is the out-of-box default until then. Re-read
   // first so a format the conversion window saved meanwhile is not clobbered.
   void MainWindow::RememberSaveFormat(std::wstring const& destinationPath)
   {
      const std::wstring extension {std::filesystem::path {destinationPath}.extension().wstring()};
      if (extension.empty())
         return;

      m_settings->load();
      m_settings->setConversionFormat(extension.substr(1));   // store dot-less, matching the picker tags
      m_settings->save();
   }

   // Like Share, the outcome is confirmed both ways: success tells the user
   // the file is really there, failure is the error.
   void MainWindow::ShowSaveAsResult(bool succeeded)
   {
      showLocalizedMessage(dialogs(),
         succeeded ? L"SaveAsDoneTitle" : L"SaveAsFailTitle",
         succeeded ? L"SaveAsDoneMessage" : L"SaveAsFailMessage",
         succeeded ? DialogSeverity::Success : DialogSeverity::Error);
   }

   // Import / Export move the library through portable .sql dump files; the
   // transfer collaborator owns the format, these handlers own the dialogue
   // with the user (picker in, summary dialog out).
   winrt::fire_and_forget MainWindow::OnMenuFileImportLibrary(IInspectable const&,RoutedEventArgs const&)
   {
      try {
         auto lifetime {get_strong()};   // keep the window alive across the co_await

         const hstring path {co_await pickDumpFileToOpen()};

         // Hop back to the UI thread — the picker can resume us elsewhere.
         co_await wil::resume_foreground(DispatcherQueue());
         if (path.empty()) co_return;

         ShowImportResult(m_libraryTransfer->importFrom(std::wstring {path}));

      } catch (const std::exception& exception) {
         anka::Core::Error::ExceptionHandler::OExceptionHandler::getInstance().handleException(exception);
      }
   }

   winrt::fire_and_forget MainWindow::OnMenuFileExportLibrary(IInspectable const&,RoutedEventArgs const&)
   {
      try {
         auto lifetime {get_strong()};   // keep the window alive across the co_await

         const hstring path {co_await pickDumpFileToSave()};

         // Hop back to the UI thread — the picker can resume us elsewhere.
         co_await wil::resume_foreground(DispatcherQueue());
         if (path.empty()) co_return;

         ShowExportResult(m_libraryTransfer->exportTo(std::wstring {path}));

      } catch (const std::exception& exception) {
         anka::Core::Error::ExceptionHandler::OExceptionHandler::getInstance().handleException(exception);
      }
   }

   winrt::Windows::Foundation::IAsyncOperation<hstring> MainWindow::pickDumpFileToOpen()
   {
      auto& picker {anka::Core::IO::OFilePicker::getInstance()};
      picker.setOwner(EnsureWindowHandle());
      co_return co_await picker.pickFile({L".sql"});
   }

   winrt::Windows::Foundation::IAsyncOperation<hstring> MainWindow::pickDumpFileToSave()
   {
      const std::wstring typeName {OLocalizationService::getInstance().text(L"LibDumpTypeName")};

      auto& picker {anka::Core::IO::OFilePicker::getInstance()};
      picker.setOwner(EnsureWindowHandle());
      co_return co_await picker.pickSaveFile(typeName, L".sql", L"Talay.library");
   }

   // Tell the user how the replay went: per-row counts on success, a plain
   // failure message when the dump could not be read or replayed.
   void MainWindow::ShowImportResult(TransferSummary const& summary)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};

      if (!summary.succeeded) {
         dialogs().showMessage({loc.text(L"LibImportTitle"), loc.text(L"LibImportFailMessage"), DialogSeverity::Error});
         return;
      }

      const std::wstring message {std::vformat(loc.text(L"LibImportDoneMessage"),
         std::make_wformat_args(summary.imported, summary.skippedExisting, summary.skippedMissing))};
      dialogs().showMessage({loc.text(L"LibImportTitle"), message, DialogSeverity::Success});
   }

   void MainWindow::ShowExportResult(bool succeeded)
   {
      showLocalizedMessage(dialogs(), L"LibExportTitle",
         succeeded ? L"LibExportDoneMessage" : L"LibExportFailMessage",
         succeeded ? DialogSeverity::Success : DialogSeverity::Error);
   }

   // Share copies the open image onto the clipboard so any other app can
   // paste it; without an image there is nothing to copy, so say that instead.
   // With a view effect on screen, the user first picks which version goes out.
   void MainWindow::OnMenuFileShare(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         ReportShareWithoutImage();
         return;
      }

      if (m_effectKind == EffectKind::None) {
         ShareImage(false);
         return;
      }

      AskShareChoice();
   }

   void MainWindow::ReportShareWithoutImage()
   {
      showLocalizedMessage(dialogs(), L"ShareNoImageTitle", L"ShareNoImageMessage", DialogSeverity::Information);
   }

   // An effect is on screen, so the clipboard copy could honestly be either
   // version: ask. Cancelling shares nothing — backing out must never be
   // mistaken for a choice.
   void MainWindow::AskShareChoice()
   {
      auto lifetime {get_strong()};   // keep the window alive until the user answers

      const OLocalizationService& loc {OLocalizationService::getInstance()};
      dialogs().askChoice(
         {loc.text(L"ShareChoiceTitle"), loc.text(L"ShareChoiceMessage"), DialogSeverity::Question},
         {loc.text(L"ChoiceWithEffect"), loc.text(L"ChoiceOriginal"), loc.text(L"ChoiceCancel")},
         [lifetime] { lifetime->ShareImage(true); },
         [lifetime] { lifetime->ShareImage(false); });
   }

   // One door for both variants: a fresh decode goes out as-is, or with the
   // active view effect baked into the clipboard copy only.
   void MainWindow::ShareImage(bool withEffect)
   {
      EnsureSharePresenter();

      // The effect may have been reset (a new image arrived over the instance
      // channel) while the user was answering; the original is then the truth.
      const bool effected {withEffect && m_effectKind != EffectKind::None};
      const bool succeeded {effected
         ? m_sharePresenter->shareFileWithEffect(m_currentImagePath, getHWND(), *m_effectCatalog->effectFor(m_effectKind))
         : m_sharePresenter->shareFile(m_currentImagePath, getHWND())};

      ShowShareResult(succeeded);
   }

   // Lazy MVP wiring: both collaborators are process-wide services, so one
   // presenter binds them once and serves every share after that.
   void MainWindow::EnsureSharePresenter()
   {
      if (m_sharePresenter)
         return;

      m_sharePresenter = std::make_unique<OSharePresenter>(
         OImageFileReader::getInstance(), OImageClipboard::getInstance());
   }

   // Unlike Print, a clipboard copy gives the user no feedback of its own,
   // so success is confirmed too — not just failure.
   void MainWindow::ShowShareResult(bool succeeded)
   {
      showLocalizedMessage(dialogs(),
         succeeded ? L"ShareDoneTitle" : L"ShareFailTitle",
         succeeded ? L"ShareDoneMessage" : L"ShareFailMessage",
         succeeded ? DialogSeverity::Success : DialogSeverity::Error);
   }

   // Print is meaningful only when an image is on screen; otherwise tell the
   // user to open one first rather than raising the printer dialog for nothing.
   void MainWindow::OnMenuFilePrint(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         showLocalizedMessage(dialogs(), L"PrintNoImageTitle", L"PrintNoImageMessage", DialogSeverity::Information);
         return;
      }

      EnsurePrintPresenter();
      ShowPrintResult(m_printPresenter->printFile(m_currentImagePath, getHWND()));
   }

   // Lazy MVP wiring: both collaborators are process-wide services, so one
   // presenter binds them once and serves every print after that.
   void MainWindow::EnsurePrintPresenter()
   {
      if (m_printPresenter)
         return;

      m_printPresenter = std::make_unique<OPrintPresenter>(
         OImageFileReader::getInstance(), OImagePrinter::getInstance());
   }

   // Quiet on success and on the user's own cancel; only a real failure
   // interrupts with a dialog.
   void MainWindow::ShowPrintResult(PrintOutcome outcome)
   {
      if (outcome != PrintOutcome::Failed)
         return;

      showLocalizedMessage(dialogs(), L"PrintFailTitle", L"PrintFailMessage", DialogSeverity::Error);
   }

   void MainWindow::OnMenuFileExit(IInspectable const&,RoutedEventArgs const&)
   {
      ConfirmExit();
   }

   // Every in-app exit path — the title-bar X and File › Exit — asks the same
   // question; Close() is only reached through the user's yes.
   void MainWindow::ConfirmExit()
   {
      auto lifetime {get_strong()};   // keep the window alive until the user answers

      const OLocalizationService& loc {OLocalizationService::getInstance()};
      dialogs().askQuestion(
         {loc.text(L"DialogExitTitle"), loc.text(L"DialogExitMessage"), DialogSeverity::Question},
         [lifetime] { lifetime->Close(); });
   }

   // ── Menu — Effects ───────────────────────────────────────────────────────

   // XAML Tag → EffectKind: the single registry both the click handler and the
   // menu sync walk, so the two can never disagree about what a radio means.
   namespace
   {
      constexpr std::pair<std::wstring_view, EffectKind> k_effectTags[] {
         {L"Grayscale", EffectKind::Grayscale}, {L"Sepia",    EffectKind::Sepia},
         {L"Invert",    EffectKind::Invert},    {L"Warm",     EffectKind::Warm},
         {L"Cool",      EffectKind::Cool},      {L"Vibrant",  EffectKind::Vibrant},
         {L"Muted",     EffectKind::Muted},     {L"Brighten", EffectKind::Brighten},
         {L"Darken",    EffectKind::Darken},    {L"Sharpen",  EffectKind::Sharpen},
         {L"Soften",    EffectKind::Soften},    {L"Vignette", EffectKind::Vignette}
      };

      EffectKind effectKindFromTag(std::wstring_view tag)
      {
         for (const auto& [name, kind] : k_effectTags) {
            if (name == tag)
               return kind;
         }

         return EffectKind::None;
      }

      hstring effectTagOf(RadioMenuFlyoutItem const& item)
      {
         return winrt::unbox_value_or<hstring>(item.Tag(), L"");
      }
   }

   // Every effect radio routes here; its Tag names the EffectKind, so a new
   // effect is one XAML line, one enum value and one catalog entry — never a
   // new handler.
   void MainWindow::OnMenuEffectPicked(IInspectable const& sender,RoutedEventArgs const&)
   {
      if (const auto item {sender.try_as<RadioMenuFlyoutItem>()})
         ApplyEffect(effectKindFromTag(effectTagOf(item)));
   }

   void MainWindow::OnMenuEffectReset(IInspectable const&,RoutedEventArgs const&)
   {
      ApplyEffect(EffectKind::None);
   }

   // Route the chosen effect into the presenter and mirror it on the menu.
   // The file and its decode never change — the effect lives only in the view,
   // which is what lets Reset restore the original instantly.
   void MainWindow::ApplyEffect(EffectKind kind)
   {
      if (m_currentImagePath.empty()) {
         RejectEffectWithoutImage();
         return;
      }

      EnsureEffectCatalog();
      m_effectKind = kind;
      m_presenter->setEffect(m_effectCatalog->effectFor(kind));
      SyncEffectMenu();
   }

   // Lazy wiring, like the other menu collaborators: one catalog serves every
   // effect choice after the first.
   void MainWindow::EnsureEffectCatalog()
   {
      if (!m_effectCatalog)
         m_effectCatalog = std::make_unique<OEffectCatalog>();
   }

   // The radio items mirror m_effectKind: walk the flyout and check exactly
   // the one whose tag names the active kind — new items sync themselves, and
   // the menu stays honest however the change came about (click, reset, new
   // image).
   void MainWindow::SyncEffectMenu()
   {
      const bool active {m_effectKind != EffectKind::None};

      for (const auto& entry : EffectsFlyout().Items()) {
         if (const auto radio {entry.try_as<RadioMenuFlyoutItem>()})
            radio.IsChecked(active && effectKindFromTag(effectTagOf(radio)) == m_effectKind);
      }
   }

   // The click already checked a radio item; the sync flips it back so the
   // menu stops advertising an effect that never applied.
   void MainWindow::RejectEffectWithoutImage()
   {
      SyncEffectMenu();
      showLocalizedMessage(dialogs(), L"EffectNoImageTitle", L"EffectNoImageMessage", DialogSeverity::Information);
   }

   // present() shows every new image effect-free; keep the cached kind and
   // the menu in step with that.
   void MainWindow::ResetEffectSelection()
   {
      m_effectKind = EffectKind::None;
      SyncEffectMenu();
   }

   // ── Menu — Options ───────────────────────────────────────────────────────

   void MainWindow::OnMenuOptionsClick(IInspectable const&,RoutedEventArgs const&)
   {
      EnsureSettingsPresenter();

      m_settingsDialog->show(Content().XamlRoot(), {
         .onApplied = [this](talay::Settings::SettingsApplyResult const& result) { OnSettingsApplied(result); },
         .onResetLibraryRequested = [this] { ConfirmLibraryReset(); }});
   }

   // Lazy MVP wiring: the presenter binds this window's settings store to a
   // shell-integration coordinator over the shared registrar; the trio is then
   // reused for every open. The coordinator is built first so it outlives the
   // presenter holding a reference to it.
   void MainWindow::EnsureSettingsPresenter()
   {
      if (m_settingsPresenter)
         return;

      m_shellIntegration = std::make_unique<talay::Shell::OShellIntegration>(
         anka::System::Shell::OShellRegistrar::getInstance());
      m_settingsPresenter = std::make_unique<talay::Settings::OSettingsPresenter>(
         *m_settings, *m_shellIntegration);
      m_settingsDialog = std::make_unique<talay::GUI::Settings::OSettingsDialog>(*m_settingsPresenter);
   }

   // Always-on-top lands immediately; a language change only takes effect on
   // the next start, so the user is offered one right away.
   void MainWindow::OnSettingsApplied(talay::Settings::SettingsApplyResult const& result)
   {
      ApplyAlwaysOnTop(result.alwaysOnTop);

      if (result.languageChanged)
         PromptRestart();
   }

   void MainWindow::ApplyAlwaysOnTop(bool enabled)
   {
      GetPresenter().IsAlwaysOnTop(enabled);
   }

   void MainWindow::PromptRestart()
   {
      auto lifetime {get_strong()};   // keep the window alive until the user answers

      const OLocalizationService& loc {OLocalizationService::getInstance()};
      dialogs().askQuestion(
         {loc.text(L"SettingsRestartTitle"), loc.text(L"SettingsRestartMessage"), DialogSeverity::Question},
         [lifetime] { lifetime->Relaunch(); });
   }

   // Start a fresh process (which reads the new language on boot) and bow out.
   // The --relaunch flag tells the successor to seize the single-instance role
   // rather than forward to this dying one — otherwise the restart leaves
   // nothing running.
   void MainWindow::Relaunch()
   {
      wchar_t exePath[MAX_PATH] {};
      GetModuleFileNameW(nullptr, exePath, MAX_PATH);

      ShellExecuteW(nullptr, L"open", exePath, std::wstring {talay::Launch::k_relaunchArg}.c_str(),
                    nullptr, SW_SHOWNORMAL);
      Close();
   }

   // The cautious half of "reset library": name the cost (entry count), make
   // clear the files stay, and only clear on an explicit Yes.
   void MainWindow::ConfirmLibraryReset()
   {
      auto lifetime {get_strong()};   // keep the window alive until the user answers

      const std::size_t count {m_library->entryCount()};
      const std::wstring message {std::vformat(
         OLocalizationService::getInstance().text(L"SettingsResetLibWarnMessage"),
         std::make_wformat_args(count))};

      dialogs().askQuestion(
         {OLocalizationService::getInstance().text(L"SettingsResetLibTitle"), message, DialogSeverity::Warning},
         [lifetime] { lifetime->ResetLibrary(); });
   }

   void MainWindow::ResetLibrary()
   {
      const bool cleared {m_library->clear()};

      showLocalizedMessage(dialogs(), L"SettingsResetLibTitle",
         cleared ? L"SettingsResetLibDoneMessage" : L"SettingsResetLibFailMessage",
         cleared ? DialogSeverity::Success : DialogSeverity::Error);
      SyncLibraryToggles();
   }

   // ── Menu — Settings › Reset All Settings ─────────────────────────────────

   void MainWindow::OnMenuSettingsResetAll(IInspectable const&, RoutedEventArgs const&)
   {
      ConfirmResetAllSettings();
   }

   // The cautious half of "reset all settings": a reset is irreversible and
   // includes the window's own size and position, so it only proceeds on Yes.
   void MainWindow::ConfirmResetAllSettings()
   {
      auto lifetime {get_strong()};   // keep the window alive until the user answers

      const OLocalizationService& loc {OLocalizationService::getInstance()};
      dialogs().askQuestion(
         {loc.text(L"SettingsResetAllTitle"), loc.text(L"SettingsResetAllMessage"), DialogSeverity::Warning},
         [lifetime] { lifetime->ResetAllSettings(); });
   }

   // Route the reset through the presenter so the registry and the live window
   // react exactly as they do after an apply, then report the outcome.
   void MainWindow::ResetAllSettings()
   {
      EnsureSettingsPresenter();
      const talay::Settings::SettingsApplyResult result {m_settingsPresenter->resetAllToDefaults()};

      ApplyAlwaysOnTop(result.alwaysOnTop);
      SyncLibraryToggles();
      AnnounceSettingsReset(result.languageChanged);
   }

   // Exactly one dialog, never two at once (WinUI allows one per XamlRoot): a
   // cleared language needs a restart to take hold, so its prompt stands in for
   // the plain "done" message.
   void MainWindow::AnnounceSettingsReset(bool languageChanged)
   {
      if (languageChanged) {
         PromptRestart();
         return;
      }

      showLocalizedMessage(dialogs(), L"SettingsResetAllTitle", L"SettingsResetAllDoneMessage", DialogSeverity::Success);
   }

   // ── Menu — Help ──────────────────────────────────────────────────────────

   void MainWindow::OnMenuHelpGuide(IInspectable const&,RoutedEventArgs const&)
   {
      GuideModal().Visibility(Visibility::Visible);
   }

   void MainWindow::OnMenuHelpSourceCode(IInspectable const&,RoutedEventArgs const&)
   {
      OpenWebLink(talay::Links::repository);
   }

   void MainWindow::OnMenuHelpAbout(IInspectable const&,RoutedEventArgs const&)
   {
      AboutModal().Visibility(Visibility::Visible);
   }

   // A refused launch must not pass silently — a menu pick that does nothing
   // reads as a dead menu, so the browser's refusal becomes an error dialog.
   void MainWindow::OpenWebLink(std::wstring const& url)
   {
      if (OLinkOpener::getInstance().openWebLink(url))
         return;

      showLocalizedMessage(dialogs(), L"ErrorTitle", L"LinkOpenFailMessage", DialogSeverity::Error);
   }

   // ── Toolbar ──────────────────────────────────────────────────────────────

   void MainWindow::OnFolderBtnClick(IInspectable const&,RoutedEventArgs const&)
   {
      SetReelCollapsed(!FolderBtn().IsChecked().Value());
      RevealReelSelection();
   }

   // Favoriting an image that is not in the library yet quietly adds it first —
   // a favorite mark with no row to live on would be lost.
   void MainWindow::OnFavBtnClick(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         FavBtn().IsChecked(false);
         return;
      }

      if (FavBtn().IsChecked().Value())
         AddCurrentToLibrary();

      m_library->setFavorite(m_currentImagePath, FavBtn().IsChecked().Value());
   }

   void MainWindow::OnLibAddBtnClick(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         LibAddBtn().IsChecked(false);
         return;
      }

      if (LibAddBtn().IsChecked().Value())
         AddCurrentToLibrary();
      else
         RemoveCurrentFromLibrary();
   }

   void MainWindow::AddCurrentToLibrary()
   {
      m_library->add(buildLibraryEntry(m_currentImagePath));
      LibAddBtn().IsChecked(true);
   }

   void MainWindow::RemoveCurrentFromLibrary()
   {
      RemoveFromLibrary(m_currentImagePath);
   }

   // A library pick means "done browsing": the panel folds away so what it
   // triggered — the viewport, or the conversion dialog — is what the user
   // sees next, not an 880px overlay covering it.
   void MainWindow::OpenFromLibrary(std::wstring const& path)
   {
      CollapseLibrary();
      DisplayImage(hstring {path});
   }

   void MainWindow::ConvertFromLibrary(std::wstring const& path)
   {
      CollapseLibrary();
      RouteConvertRequest({path});
   }

   // Panel context menus remove arbitrary rows; when the removed row is the
   // image on screen, the toolbar toggles resync so they stop advertising a
   // library entry (and its favorite mark) that no longer exists.
   void MainWindow::RemoveFromLibrary(std::wstring const& path)
   {
      if (!m_library->remove(path)) {
         ReportLibraryUpdateFailure();
         return;
      }

      if (path == m_currentImagePath)
         SyncLibraryToggles();
   }

   // Same resync rule for dropping a favorite mark from the panel: the
   // toolbar star follows the library when the on-screen image is the target.
   void MainWindow::UnfavoriteInLibrary(std::wstring const& path)
   {
      if (!m_library->setFavorite(path, false)) {
         ReportLibraryUpdateFailure();
         return;
      }

      if (path == m_currentImagePath)
         SyncLibraryToggles();
   }

   // The repository signals failure through its return value; a menu pick
   // that silently did nothing would read as a dead menu, so it gets a dialog.
   void MainWindow::ReportLibraryUpdateFailure()
   {
      showLocalizedMessage(dialogs(), L"ErrorTitle", L"LibUpdateFailMessage", DialogSeverity::Error);
   }

   // File info is meaningful only when an image is on screen; otherwise tell
   // the user to open one first — the same rule File › Convert applies.
   void MainWindow::OnInfoBtnClick(IInspectable const&,RoutedEventArgs const&)
   {
      if (m_currentImagePath.empty()) {
         RejectInfoWithoutImage();
         return;
      }

      m_infoOpen = InfoBtn().IsChecked().Value();
      InfoModal().Visibility(m_infoOpen ? Visibility::Visible : Visibility::Collapsed);
   }

   // The click already checked the toggle; flip it back so the button stops
   // advertising a modal that never opened.
   void MainWindow::RejectInfoWithoutImage()
   {
      InfoBtn().IsChecked(false);
      showLocalizedMessage(dialogs(), L"InfoNoImageTitle", L"InfoNoImageMessage", DialogSeverity::Information);
   }

   void MainWindow::OnRotateBtnClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_viewport->rotateClockwise();
   }

   void MainWindow::OnZoomInClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_viewport->zoomBy(1.15);
   }

   void MainWindow::OnZoomOutClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_viewport->zoomBy(1.0 / 1.15);
   }

   // User-driven slider moves drive the viewport; programmatic syncs (guarded)
   // are ignored so the viewport→slider→viewport loop never forms.
   void MainWindow::OnZoomSliderChanged(IInspectable const&,RangeBaseValueChangedEventArgs const& e)
   {
      if (m_syncingZoom || !m_viewport)
         return;

      m_viewport->setZoom(e.NewValue());
   }

   void MainWindow::OnFullscreenBtnClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_fullscreen = FsBtn().IsChecked().Value();
      ApplyFullscreen(m_fullscreen);
   }

   // ── Library panel ────────────────────────────────────────────────────────

   void MainWindow::OnLibHandleClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_libOpen = !m_libOpen;
      AnimateLibrary(m_libOpen);
   }

   void MainWindow::OnLibCloseClick(IInspectable const&,RoutedEventArgs const&)
   {
      CollapseLibrary();
   }

   // Single authority for folding the panel away: the close button and every
   // library pick that moves the user's attention elsewhere funnel here.
   void MainWindow::CollapseLibrary()
   {
      if (!m_libOpen)
         return;

      m_libOpen = false;
      AnimateLibrary(false);
   }

   void MainWindow::OnLibResetFilters(IInspectable const&,RoutedEventArgs const&)
   {
      m_libraryPanel->resetFilters();
   }

   // ── Reel panel ───────────────────────────────────────────────────────────

   void MainWindow::OnReelCollapseClick(IInspectable const&,RoutedEventArgs const&)
   {
      SetReelCollapsed(!m_reelCollapsed);
      RevealReelSelection();
   }

   // A manual expand shows the strip again with whatever scroll offset it last
   // had; re-showing the open image re-checks its tile and scrolls it into
   // view. Only for user-driven expands — the auto-reveal on a new folder is
   // already inside a show() that ends with the same scroll.
   void MainWindow::RevealReelSelection()
   {
      if (!m_reelCollapsed && !m_currentImagePath.empty())
         m_reel->show(m_currentImagePath);
   }

   // ── Info modal ───────────────────────────────────────────────────────────

   void MainWindow::OnInfoCloseClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_infoOpen = false;
      InfoModal().Visibility(Visibility::Collapsed);
      InfoBtn().IsChecked(false);
   }

   void MainWindow::OnInfoModalBackdropClick(IInspectable const&,RoutedEventArgs const&)
   {
      OnInfoCloseClick(nullptr,RoutedEventArgs {});
   }

   // ── Guide modal ──────────────────────────────────────────────────────────

   void MainWindow::OnGuideCloseClick(IInspectable const&,RoutedEventArgs const&)
   {
      GuideModal().Visibility(Visibility::Collapsed);
   }

   void MainWindow::OnGuideModalBackdropClick(IInspectable const&,RoutedEventArgs const&)
   {
      OnGuideCloseClick(nullptr,RoutedEventArgs {});
   }

   // ── About modal ──────────────────────────────────────────────────────────

   void MainWindow::OnAboutCloseClick(IInspectable const&,RoutedEventArgs const&)
   {
      AboutModal().Visibility(Visibility::Collapsed);
   }

   void MainWindow::OnAboutModalBackdropClick(IInspectable const&,RoutedEventArgs const&)
   {
      OnAboutCloseClick(nullptr,RoutedEventArgs {});
   }

   void MainWindow::OnAboutHomepageClick(IInspectable const&,RoutedEventArgs const&)
   {
      OpenWebLink(talay::Links::homepage);
   }

   // ── Fullscreen exit ───────────────────────────────────────────────────────

   void MainWindow::OnFsExitClick(IInspectable const&,RoutedEventArgs const&)
   {
      m_fullscreen = false;
      ApplyFullscreen(false);
      FsBtn().IsChecked(false);
   }

   // ── Viewport clip ────────────────────────────────────────────────────────

   void MainWindow::OnViewportSizeChanged(IInspectable const&,SizeChangedEventArgs const& e)
   {
      auto sz = e.NewSize();
      auto geom = RectangleGeometry {};
      geom.Rect({0.f,0.f,static_cast<float>(sz.Width),static_cast<float>(sz.Height)});
      ViewportGrid().Clip(geom);
   }

   // ── Image presentation ─────────────────────────────────────────────────────

   // Bring the Direct2D stack up once for the window's lifetime, then create the
   // presenter that draws opened images into RenderImage on top of it.
   void MainWindow::InitializeRenderer()
   {
      ORenderer::getInstance().initialize();
      m_presenter = std::make_unique<OImagePresenter>();
      m_presenter->attach(RenderImage());
   }

   // Decode the file, build a still or animation from it, and present it; report
   // a failure rather than leaving a stale image on screen.
   void MainWindow::DisplayImage(hstring const& path)
   {
      const ImageSequence sequence = OImageFileReader::getInstance().readSequence(std::wstring {path});
      const bool shown = m_presenter->present(sequence);

      ResetEffectSelection();   // present() dropped any view effect — mirror that
      if (!shown) {
         ConcealViewport();
         ReportImageOpenFailure();
         return;
      }

      RevealViewport();
      PopulateImageInfo(path);
      TrackOpenedImage(std::wstring {path});
   }

   void MainWindow::ReportImageOpenFailure()
   {
      showLocalizedMessage(dialogs(), L"ErrorTitle", L"ImageOpenFailed", DialogSeverity::Error);
   }

   // Every successful open updates the surfaces that remember it: the reel,
   // the recent files list, the (auto-add) library and its toolbar toggles.
   void MainWindow::TrackOpenedImage(std::wstring const& path)
   {
      m_currentImagePath = path;
      m_reel->show(m_currentImagePath);
      m_recentFiles->record(m_currentImagePath);
      AutoAddToLibrary();
      SyncLibraryToggles();
   }

   // Honour the auto-add setting: every successfully opened image lands in the
   // library. An image already there makes add() a no-op, and the toggle sync
   // that follows reflects the new state on the toolbar.
   void MainWindow::AutoAddToLibrary()
   {
      if (m_settings->isAutoAddToLibraryEnabled())
         m_library->add(buildLibraryEntry(m_currentImagePath));
   }

   // Reflect the opened image's library state on the toolbar toggles.
   void MainWindow::SyncLibraryToggles()
   {
      LibAddBtn().IsChecked(m_library->contains(m_currentImagePath));
      FavBtn().IsChecked(m_library->isFavorite(m_currentImagePath));
   }

   // Read the file's metadata once and fan it out to the info modal, the toolbar
   // chips and the viewport HUD, so every "file info" surface shows the open image.
   void MainWindow::PopulateImageInfo(hstring const& path)
   {
      const ImageMetadata meta = OImageMetadataReader::getInstance().read(std::wstring {path});

      InfoPreview().Source(m_presenter->canvas());   // share the rendered bitmap as the panel preview
      UpdateInfoModal(meta);
      UpdateInfoChips(meta);
      UpdateInfoHud(meta);
   }

   void MainWindow::UpdateInfoModal(ImageMetadata const& meta)
   {
      UpdateInfoIdentity(meta);
      UpdateInfoGeometry(meta);
      UpdateInfoFileFacts(meta);
   }

   void MainWindow::UpdateInfoIdentity(ImageMetadata const& meta)
   {
      InfoValName().Text(InfoText::orDash(meta.fileName));
      InfoValFormat().Text(InfoText::orDash(meta.format));
      InfoValPath().Text(InfoText::orDash(meta.filePath));
   }

   void MainWindow::UpdateInfoGeometry(ImageMetadata const& meta)
   {
      InfoValDims().Text(InfoText::pixelDimensions(meta));
      InfoValAspect().Text(InfoText::aspectRatio(meta));
      InfoValDepth().Text(InfoText::colorDepth(meta));
      InfoValProfile().Text(InfoText::orDash(meta.colorProfile));
      InfoValDpi().Text(InfoText::resolution(meta));
   }

   void MainWindow::UpdateInfoFileFacts(ImageMetadata const& meta)
   {
      InfoValSize().Text(InfoText::fileSize(meta.fileSizeBytes));
      InfoValCreated().Text(InfoText::orDash(meta.created));
      InfoValModified().Text(InfoText::orDash(meta.modified));
   }

   // The two toolbar chips the user reads at a glance: pixel dimensions and the
   // on-disk size in whole kilobytes.
   void MainWindow::UpdateInfoChips(ImageMetadata const& meta)
   {
      ChipDimensions().Text(InfoText::dimensions(meta));
      ChipFileSize().Text(InfoText::kilobytes(meta.fileSizeBytes));
   }

   void MainWindow::UpdateInfoHud(ImageMetadata const& meta)
   {
      HudFormat().Text(InfoText::orDash(meta.format));
      HudDimensions().Text(InfoText::dimensions(meta));
   }

   // Collapse the empty-state placeholder and frame the freshly opened image.
   void MainWindow::RevealViewport()
   {
      ViewportPlaceholder().Visibility(Visibility::Collapsed);
      FrameOpenedImage();
   }

   // A failed present leaves nothing shown (the presenter cleared itself), so
   // the open-image state and the empty-state placeholder must say the same —
   // Convert, Share, Print and Effects then honestly ask for an image first.
   void MainWindow::ConcealViewport()
   {
      m_currentImagePath.clear();
      ViewportPlaceholder().Visibility(Visibility::Visible);
   }

   // Hand the presented canvas's pixel size to the viewport so it can frame
   // the image: original size when it fits, scaled down to fit when it
   // overflows — the zoom readout follows through the viewport's callback.
   void MainWindow::FrameOpenedImage()
   {
      if (const auto canvas = m_presenter->canvas())
         m_viewport->frameContent({static_cast<double>(canvas.PixelWidth()),
                                   static_cast<double>(canvas.PixelHeight())});
   }

   // ── Folder reel ─────────────────────────────────────────────────────────────

   // Create the reel collaborator, bind it to its XAML controls (strip, pager
   // bar, themed-resource host), and wire its callbacks: a thumbnail click opens
   // that image; a folder change asks us to reveal the (possibly collapsed) panel.
   void MainWindow::InitializeReel()
   {
      m_reel = std::make_unique<OFolderReel>(talay::Formats::supportedImageFormats());
      m_reel->attach(CollectReelControls());
      m_reel->onImageActivated([this](std::wstring const& path) { DisplayImage(hstring {path}); });
      m_reel->onExpandRequested([this] { EnsureReelExpanded(); });
   }

   ReelControls MainWindow::CollectReelControls()
   {
      ReelControls controls;
      controls.strip        = ReelThumbs();
      controls.scroller     = ReelContent();
      controls.pagePrev     = ReelPagePrev();
      controls.pageNext     = ReelPageNext();
      controls.pageLabel    = ReelPageLabel();
      controls.resourceHost = RootGrid();
      return controls;
   }

   // Slide the reel open the first time images appear, so the thumbnails are seen
   // without the user having to expand the panel.
   void MainWindow::EnsureReelExpanded()
   {
      if (m_reelCollapsed) {
         SetReelCollapsed(false);
      }
   }

   // Single authority for the reel's open/closed state: every entry point (the
   // toolbar toggle, the panel chevron, the auto-reveal on a new folder) funnels
   // here, so the panel animation and the toolbar toggle can never drift apart.
   void MainWindow::SetReelCollapsed(bool collapse)
   {
      m_reelCollapsed = collapse;
      AnimateReel(collapse);
      FolderBtn().IsChecked(!collapse);
   }

   // ── Image library ───────────────────────────────────────────────────────────

   // Stand up the SQLite-backed repository (database lives next to the settings
   // file), the panel controller, and the dump transfer, then wire the panel's
   // entry requests (open / convert / unfavorite / remove) back to us.
   void MainWindow::InitializeLibrary()
   {
      m_library = std::make_unique<OImageLibrary>(
         std::make_unique<OSqliteLibraryStore>(std::make_unique<OSqliteDatabase>()));
      m_library->open((anka::System::Process::executableDirectory() / L"Talay.library.db").wstring());

      m_libraryPanel = std::make_unique<OLibraryPanel>(*m_library);
      m_libraryPanel->onOpenRequested([this](std::wstring const& path) { OpenFromLibrary(path); });
      m_libraryPanel->onRemoveRequested([this](std::wstring const& path) { RemoveFromLibrary(path); });
      m_libraryPanel->onConvertRequested([this](std::wstring const& path) { ConvertFromLibrary(path); });
      m_libraryPanel->onUnfavoriteRequested([this](std::wstring const& path) { UnfavoriteInLibrary(path); });
      m_libraryPanel->attach(CollectLibraryControls());

      m_libraryTransfer = std::make_unique<OLibraryTransfer>(
         *m_library, [] { return std::make_unique<OSqliteDatabase>(); });

      PruneMissingLibraryEntries();
   }

   // Launch the existence sweep: entries whose backing file was deleted or
   // renamed since they were added stop being listed. The disk I/O runs off
   // the UI thread; the removal is marshalled back here, guarded by the
   // window's lifetime so a sweep that lands after close quietly does nothing.
   void MainWindow::PruneMissingLibraryEntries()
   {
      m_libraryReconciler = std::make_unique<OLibraryReconciler>(
         *m_library,
         [weak = get_weak(), queue = DispatcherQueue()](std::function<void(void)> work) {
            queue.TryEnqueue([weak, work = std::move(work)] {
               if (weak.get()) work();
            });
         });

      m_libraryReconciler->pruneMissing();
   }

   // ── Recent files ────────────────────────────────────────────────────────────

   // Stand up the tracker over its own connection to the library's database
   // file (a separate recent_file table; opening prunes entries whose file is
   // gone), then bind the menu presenter to the File › Recent Files submenu.
   void MainWindow::InitializeRecentFiles()
   {
      m_recentFiles = std::make_unique<ORecentFilesTracker>(
         std::make_unique<OSqliteRecentFilesStore>(std::make_unique<OSqliteDatabase>()),
         k_recentFilesCapacity);
      m_recentFiles->open((anka::System::Process::executableDirectory() / L"Talay.library.db").wstring());

      m_recentFilesMenu = std::make_unique<ORecentFilesMenu>(*m_recentFiles);
      m_recentFilesMenu->onOpenRequested([this](std::wstring const& path) { OpenRecentFile(path); });
      m_recentFilesMenu->attach(FileRecentSubMenu());
   }

   // A recent entry can outlive its file: re-check on the click, and when the
   // file is gone let the tracker drop it (the menu rebuilds itself) and say so.
   void MainWindow::OpenRecentFile(std::wstring const& path)
   {
      if (!m_recentFiles->revalidate(path)) {
         ReportRecentFileMissing();
         return;
      }

      DisplayImage(hstring {path});
   }

   void MainWindow::ReportRecentFileMissing()
   {
      showLocalizedMessage(dialogs(), L"RecentFileMissingTitle", L"RecentFileMissingMessage", DialogSeverity::Information);
   }

   LibraryPanelControls MainWindow::CollectLibraryControls()
   {
      LibraryPanelControls controls;
      CollectTextFilterControls(controls);
      CollectRangeFilterControls(controls);
      CollectFlagFilterControls(controls);
      CollectResultControls(controls);
      CollectPagerControls(controls);
      controls.resourceHost = RootGrid();
      return controls;
   }

   void MainWindow::CollectTextFilterControls(LibraryPanelControls& controls)
   {
      controls.nameBox        = LibFilterName();
      controls.formatDropDown = LibFormatDropDown();
      controls.formatSummary  = LibFormatSummary();
      controls.formatList     = LibFormatList();
      controls.folderBox      = LibFilterFolder();
   }

   void MainWindow::CollectRangeFilterControls(LibraryPanelControls& controls)
   {
      controls.dateFrom  = LibFilterDateFrom();
      controls.dateTo    = LibFilterDateTo();
      controls.sizeMin   = LibFilterSizeMin();
      controls.sizeMax   = LibFilterSizeMax();
      controls.widthMin  = LibFilterWidthMin();
      controls.widthMax  = LibFilterWidthMax();
      controls.heightMin = LibFilterHeightMin();
      controls.heightMax = LibFilterHeightMax();
   }

   void MainWindow::CollectFlagFilterControls(LibraryPanelControls& controls)
   {
      controls.stills        = LibFilterStills();
      controls.animated      = LibFilterAnimated();
      controls.favoritesOnly = LibFilterFavorite();
   }

   void MainWindow::CollectResultControls(LibraryPanelControls& controls)
   {
      controls.displayMode = LibDisplayMode();
      controls.sortMode    = LibSortMode();
      controls.itemCount   = LibItemCount();
      controls.resultCount = LibResultCount();
      controls.emptyLabel  = LibEmptyLabel();
      controls.tableView   = LibTableView();
      controls.tableRows   = LibTableRows();
      controls.thumbGrid   = LibThumbGrid();
   }

   void MainWindow::CollectPagerControls(LibraryPanelControls& controls)
   {
      controls.pageSize  = LibPageSize();
      controls.pagePrev  = LibPagePrev();
      controls.pageNext  = LibPageNext();
      controls.pageLabel = LibPageLabel();
   }

   // ── Private helpers ───────────────────────────────────────────────────────

   // Runs a one-shot DoubleAnimation (EaseOut) on a single property via a throwaway
   // Storyboard. dependent=true is required for layout properties such as Width,
   // which the animation system otherwise ignores.
   void MainWindow::AnimateTo(Microsoft::UI::Xaml::DependencyObject const& target,
                              hstring const& property, double to, int durationMs, bool dependent)
   {
      auto sb = Storyboard {};
      auto anim = DoubleAnimation {};
      anim.EnableDependentAnimation(dependent);
      anim.To(to);
      anim.Duration(MS(durationMs));
      anim.EasingFunction(EaseOut());
      Storyboard::SetTarget(anim,target);
      Storyboard::SetTargetProperty(anim,property);
      sb.Children().Append(anim);
      sb.Begin();
   }

   void MainWindow::AnimateLibrary(bool open)
   {
      AnimateTo(LibPanelTranslate(),L"X",open ? 0.0 : -(k_libPanelWidth + 3.0),380,false);
      AnimateTo(LibHandleChevronRotate(),L"Angle",open ? 180.0 : 0.0,280,false);
   }

   void MainWindow::AnimateReel(bool collapse)
   {
      // Width is a layout property, so the move animation must be dependent.
      AnimateTo(ReelPanel(),L"Width",collapse ? k_reelCollapsedWidth : k_reelExpandedWidth,320,true);

      ReelContent().Visibility(collapse ? Visibility::Collapsed : Visibility::Visible);
      ReelTitle().Visibility(collapse ? Visibility::Collapsed : Visibility::Visible);
      ReelPager().Visibility(collapse ? Visibility::Collapsed : Visibility::Visible);
      // Chevron points the way the panel will go: ">" to collapse right, "<" to expand left
      ReelCollapseIcon().Glyph(collapse ? L"\uE76B" : L"\uE76C");
   }

   // Build the transform controller, bind it to the image (the element it
   // transforms) and the render border (the surface it listens on for drags),
   // and feed every zoom change back to the toolbar.
   void MainWindow::InitializeViewport()
   {
      m_viewport = std::make_unique<anka::GUI::Viewport::OImageViewport>(
         anka::GUI::Viewport::ZoomRange {k_zoomMin, k_zoomMax});

      m_viewport->onZoomChanged([this](double percent) {
         SyncZoomControls(percent);
         if (m_cursorController)
            m_cursorController->refresh();   // a zoom can make the image newly (un)draggable
      });
      m_viewport->attach(RenderImage(), RenderBorder());
   }

   // Slip an OCursorHost between the window and its content so one cursor can
   // serve the whole tree (a descendant that sets its own still wins), then let
   // the controller drive it: a move cursor while the image is large enough to
   // pan, a hand over anything clickable. Rules are consulted in order.
   void MainWindow::InitializeCursors()
   {
      const winrt::Talay::OCursorHost host {WrapContentInCursorHost()};

      std::vector<std::unique_ptr<ICursorRule>> rules;
      rules.push_back(std::make_unique<OViewportDragCursorRule>(
         RenderBorder(), [this] { return m_viewport->isContentDraggable(); }));
      rules.push_back(std::make_unique<OClickableCursorRule>());

      OCursorHost* sink {winrt::get_self<OCursorHost>(host)};
      m_cursorController = std::make_unique<OHoverCursorController>(
         host, std::move(rules), [sink](CursorShape shape) { sink->showCursor(shape); });
   }

   // Re-root the window's content under a fresh cursor host; the existing tree
   // keeps its place beneath it. Authored in code (not markup) so the XAML type
   // generator never needs the host's header.
   winrt::Talay::OCursorHost MainWindow::WrapContentInCursorHost()
   {
      const auto content {Content()};
      winrt::Talay::OCursorHost host {};

      Content(nullptr);
      host.Children().Append(content);
      Content(host);
      return host;
   }

   // Make the whole viewport a drop target: a supported image dragged from the
   // shell opens exactly as if it came through File › Open — unsupported files
   // are quietly refused by the target's extension filter.
   void MainWindow::InitializeDropTarget()
   {
      m_dropTarget = std::make_unique<anka::GUI::DragDrop::OFileDropTarget>();

      m_dropTarget->allowExtensions(talay::Formats::supportedImageFormats());
      m_dropTarget->onFileDropped([this](std::wstring const& path) { DisplayImage(hstring {path}); });
      m_dropTarget->attach(ViewportGrid());
   }

   // Mirror the viewport's zoom onto the slider and readout. The guard makes the
   // slider write inert so it doesn't bounce back into the viewport.
   void MainWindow::SyncZoomControls(double percent)
   {
      m_syncingZoom = true;
      ZoomSlider().Value(percent);
      ZoomValueText().Text(std::to_wstring(static_cast<int>(std::round(percent))) + L"%");
      m_syncingZoom = false;
   }

   void MainWindow::ApplyFullscreen(bool enter)
   {
      SetFullscreenPresenter(enter);
      ToggleChromeForFullscreen(enter);
      ApplyFullscreenFraming(enter);
   }

   void MainWindow::SetFullscreenPresenter(bool enter)
   {
      // Real OS-level fullscreen: window fills the monitor, no chrome.
      if (enter) {
         AppWindow().SetPresenter(AppWindowPresenterKind::FullScreen);
         return;
      }

      AppWindow().SetPresenter(AppWindowPresenterKind::Overlapped);
      // A fresh Overlapped presenter restores the native title bar — strip it
      // again, and re-apply the always-on-top preference it also forgot.
      GetPresenter().SetBorderAndTitleBar(true,false);
      ApplyAlwaysOnTop(m_settings->isAlwaysOnTopEnabled());
   }

   // Collapse the title-bar and toolbar rows so only the viewport (row 1, "*") shows.
   void MainWindow::ToggleChromeForFullscreen(bool enter)
   {
      TitleBarRow().Height(GridLengthHelper::FromPixels(enter ? 0.0 : 56.0));
      ToolbarRow().Height(GridLengthHelper::FromPixels(enter ? 0.0 : 74.0));
      TitleBarArea().Visibility(enter ? Visibility::Collapsed : Visibility::Visible);
      ToolbarGrid().Visibility(enter ? Visibility::Collapsed : Visibility::Visible);
      ReelPanel().Visibility(enter ? Visibility::Collapsed : Visibility::Visible);
      LibraryHandle().Visibility(enter ? Visibility::Collapsed : Visibility::Visible);
      FsExitBtn().Visibility(enter ? Visibility::Visible : Visibility::Collapsed);
   }

   // Drop the framing margins / rounded border so the image is edge-to-edge.
   void MainWindow::ApplyFullscreenFraming(bool enter)
   {
      BodyGrid().Margin(enter ? Thickness {0,0,0,0} : Thickness {12,12,12,0});
      ViewportGrid().Margin(enter ? Thickness {0,0,0,0} : Thickness {0,0,12,0});
      RenderBorder().CornerRadius(enter ? CornerRadius {0,0,0,0} : CornerRadius {12,12,12,12});
      RenderBorder().BorderThickness(enter ? Thickness {0,0,0,0} : Thickness {1,1,1,1});
   }

   Microsoft::UI::Xaml::Media::Animation::DoubleAnimation MainWindow::MakePulseAnimation()
   {
      auto anim = DoubleAnimation {};
      anim.From(1.0);
      anim.To(0.25);
      anim.Duration(MS(1500));
      auto ease = SineEase {};
      ease.EasingMode(EasingMode::EaseInOut);
      anim.EasingFunction(ease);
      return anim;
   }

   void MainWindow::StartEmblemPulse()
   {
      auto sb = Storyboard {};
      sb.AutoReverse(true);
      sb.RepeatBehavior(RepeatBehaviorHelper::Forever());
      auto anim = MakePulseAnimation();
      Storyboard::SetTarget(anim,EmblemGoldDot());
      Storyboard::SetTargetProperty(anim,L"Opacity");
      sb.Children().Append(anim);
      sb.Begin();
   }

   // Hands back the shared dialog service with this window's XamlRoot set as the
   // anchor, refreshed each call so it always targets the live content.
   anka::GUI::Dialog::IDialogService& MainWindow::dialogs()
   {
      auto& service {ODialogService::getInstance()};
      service.setHost(Content().XamlRoot());
      return service;
   }

   OverlappedPresenter MainWindow::GetPresenter()
   {
      return AppWindow().Presenter().as<OverlappedPresenter>();
   }

   HWND MainWindow::getHWND() const
   {
      HWND hwnd {nullptr};
      winrt::check_hresult(this->try_as<::IWindowNative>()->get_WindowHandle(&hwnd));

      return hwnd;
   }

   // Returns the live window handle, or throws if it cannot be resolved so the
   // caller (OnMenuFileOpen) can report it rather than crashing the picker.
   HWND MainWindow::EnsureWindowHandle()
   {
      HWND hwnd {getHWND()};
      if (!hwnd)
         throw std::make_shared<anka::Core::Error::Exception::OException>(
            L"MainWindowHwndCantGetException",
            L"Can't get MainWindow handle! Retry to open the file!",nullptr);

      return hwnd;
   }
}
