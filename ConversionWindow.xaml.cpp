#include "pch.h"
#include "ConversionWindow.xaml.h"
#include "OTalaySettings.hpp"
#include "LocalizedDialog.hpp"
#include "framework/GUI/Dialog/ODialogService.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"
#include "framework/Graphics/Conversion/OImageConverter.hpp"
#include "TalayPaths.hpp"

#include <shtypes.h>
#include <shobjidl_core.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <format>
#include <memory>
#include <string>
#include <vector>
#if __has_include("ConversionWindow.g.cpp")
#include "ConversionWindow.g.cpp"
#endif

namespace winrt
{
   using namespace Microsoft::UI::Xaml;
   using namespace Microsoft::UI::Xaml::Controls;
   using namespace Microsoft::UI::Windowing;
}

using winrt::Windows::Foundation::IInspectable;
using winrt::hstring;

using anka::Core::Localization::OLocalizationService;
using anka::GUI::Dialog::DialogSeverity;
using anka::GUI::Dialog::ODialogService;
using anka::Graphics::Conversion::ConversionRequest;
using anka::Graphics::Conversion::ConversionResult;
using anka::Graphics::Conversion::OImageConverter;
using anka::Graphics::Conversion::WritableFormat;
using talay::Settings::OTalaySettings;

// Win32 plumbing kept file-local, so the shell headers never leak past this unit.
namespace
{
   // The native "pick a folder" dialog, configured for filesystem folders only.
   winrt::com_ptr<IFileOpenDialog> makeFolderDialog()
   {
      winrt::com_ptr<IFileOpenDialog> dialog;
      winrt::check_hresult(CoCreateInstance(CLSID_FileOpenDialog, nullptr,
                                            CLSCTX_INPROC_SERVER, IID_PPV_ARGS(dialog.put())));

      FILEOPENDIALOGOPTIONS options {};
      dialog->GetOptions(&options);
      dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
      return dialog;
   }

   std::wstring shellItemPath(IShellItem* item)
   {
      PWSTR raw {nullptr};
      winrt::check_hresult(item->GetDisplayName(SIGDN_FILESYSPATH, &raw));

      std::wstring path {raw};
      CoTaskMemFree(raw);
      return path;
   }

   // Show the folder picker modally over 'owner'; empty string if the user cancels.
   std::wstring pickFolder(HWND owner)
   {
      winrt::com_ptr<IFileOpenDialog> dialog {makeFolderDialog()};
      if (dialog->Show(owner) != S_OK) {
         return {};
      }

      winrt::com_ptr<IShellItem> item;
      winrt::check_hresult(dialog->GetResult(item.put()));
      return shellItemPath(item.get());
   }
}

namespace winrt::Talay::implementation
{
   ConversionWindow::ConversionWindow()
   {
      InitializeComponent();
      ConfigureTitleBar();
      ConfigurePresenter();
      Closed({this, &ConversionWindow::OnWindowClosed});
   }

   // Mirror MainWindow's chrome: strip the native caption (keep the resize border)
   // and register our own drag region.
   void ConversionWindow::ConfigureTitleBar()
   {
      GetPresenter().SetBorderAndTitleBar(true, false);
      ExtendsContentIntoTitleBar(true);
      SetTitleBar(TitleBarDragArea());
   }

   // A fixed-size dialog: no resize, maximise or minimise — it behaves like a modal.
   void ConversionWindow::ConfigurePresenter()
   {
      auto presenter = GetPresenter();
      presenter.IsResizable(false);
      presenter.IsMaximizable(false);
      presenter.IsMinimizable(false);
   }

   // One-shot setup after construction: bind sources + owner, own the owner so we
   // stack above it, restore saved choices, prefill the name and go modal.
   void ConversionWindow::configure(std::vector<std::wstring> const& sourcePaths, HWND owner,
                                    std::vector<std::wstring> const& supportedFormats)
   {
      m_sourcePaths      = sourcePaths;
      m_supportedFormats = supportedFormats;
      m_owner            = owner;

      if (m_owner) {
         ::SetWindowLongPtrW(getHWND(), GWLP_HWNDPARENT, reinterpret_cast<LONG_PTR>(m_owner));
      }

      BuildFormatList();
      LoadPreferences();
      PrefillFromSource();
      RefreshSourceSummary();
      CenterOnOwner();
      DisableOwner(true);
   }

   void ConversionWindow::appendSources(std::vector<std::wstring> const& sourcePaths)
   {
      for (std::wstring const& path : sourcePaths) {
         if (std::find(m_sourcePaths.begin(), m_sourcePaths.end(), path) == m_sourcePaths.end()) {
            m_sourcePaths.push_back(path);
         }
      }

      RefreshSourceSummary();
      Activate();
   }

   // Fill the format dropdown with the writable subset of the viewer's supported
   // formats, asking the converter what its encoder chain can actually produce — so
   // the list never offers a target that would fail (Tag = request extension).
   void ConversionWindow::BuildFormatList()
   {
      auto items = FormatBox().Items();
      items.Clear();

      for (const WritableFormat& format : OImageConverter::getInstance().writableFormats(m_supportedFormats)) {
         ComboBoxItem item;
         item.Content(box_value(hstring {format.label}));
         item.Tag(box_value(hstring {format.extension}));
         items.Append(item);
      }
   }

   // Read the saved format + output directory from the ini and reflect them in the
   // controls; an absent directory falls back to the source image's own folder.
   void ConversionWindow::LoadPreferences()
   {
      m_settings = std::make_unique<OTalaySettings>(L"OTalaySettings", talay::Paths::userDataDirectory());
      m_settings->load();

      selectFormat(m_settings->getConversionFormat());

      const std::wstring savedDir {m_settings->getConversionOutputDirectory()};
      OutputDirBox().Text(hstring {savedDir.empty() ? sourceDirectory() : savedDir});
   }

   // Select the combo entry whose Tag matches the saved extension; default to the
   // first format when nothing (or nothing recognised) was saved.
   void ConversionWindow::selectFormat(std::wstring const& extension)
   {
      auto items = FormatBox().Items();
      for (uint32_t i = 0; i < items.Size(); ++i) {
         auto tag = items.GetAt(i).as<ComboBoxItem>().Tag();
         if (tag && std::wstring {unbox_value<hstring>(tag)} == extension) {
            FormatBox().SelectedIndex(static_cast<int32_t>(i));
            return;
         }
      }
      FormatBox().SelectedIndex(0);
   }

   void ConversionWindow::PrefillFromSource()
   {
      if (!isBatch() && !m_sourcePaths.empty()) {
         NameBox().Text(hstring {std::filesystem::path {m_sourcePaths.front()}.stem().wstring()});
      }
   }

   // ── Source presentation ────────────────────────────────────────────────────

   bool ConversionWindow::isBatch() const
   {
      return m_sourcePaths.size() > 1;
   }

   void ConversionWindow::RefreshSourceSummary()
   {
      SourceBox().Text(hstring {sourceSummary()});
      ToolTipService::SetToolTip(SourceBox(), box_value(hstring {joinedSources()}));
      SyncNameBoxMode();
   }

   // A batch cannot honour one custom name, so the box is parked with an
   // explanatory placeholder instead of silently ignoring the user's input.
   void ConversionWindow::SyncNameBoxMode()
   {
      NameBox().IsEnabled(!isBatch());

      if (isBatch()) {
         NameBox().Text(hstring {OLocalizationService::getInstance().text(L"ConvNameBatch")});
      }
   }

   std::wstring ConversionWindow::sourceSummary() const
   {
      if (!isBatch()) {
         return m_sourcePaths.empty() ? std::wstring {} : m_sourcePaths.front();
      }

      const std::size_t count {m_sourcePaths.size()};
      return std::vformat(OLocalizationService::getInstance().text(L"ConvSourceCount"),
                          std::make_wformat_args(count));
   }

   std::wstring ConversionWindow::joinedSources() const
   {
      std::wstring joined;
      for (std::wstring const& path : m_sourcePaths) {
         joined += (joined.empty() ? L"" : L"\n") + path;
      }

      return joined;
   }

   // Centre the window over the owner, or over the primary monitor's work area when
   // opened stand-alone. Sizes are taken as physical pixels (a small DPI shortcut).
   void ConversionWindow::CenterOnOwner()
   {
      RECT area {};
      if (!(m_owner && ::GetWindowRect(m_owner, &area))) {
         ::SystemParametersInfoW(SPI_GETWORKAREA, 0, &area, 0);
      }

      const int x = area.left + ((area.right - area.left) - k_windowWidth) / 2;
      const int y = area.top + ((area.bottom - area.top) - k_windowHeight) / 2;
      AppWindow().MoveAndResize({x, y, k_windowWidth, k_windowHeight});
   }

   // Block all input to the owner while the dialog is up (Win32 EnableWindow), and
   // hand focus back to it on close — the classic owned-modal behaviour.
   void ConversionWindow::DisableOwner(bool disable)
   {
      if (!m_owner) {
         return;
      }

      ::EnableWindow(m_owner, disable ? FALSE : TRUE);
      if (!disable) {
         ::SetForegroundWindow(m_owner);
      }
   }

   void ConversionWindow::OnWindowClosed(IInspectable const&, WindowEventArgs const&)
   {
      DisableOwner(false);
   }

   // ── Title bar ──────────────────────────────────────────────────────────────

   void ConversionWindow::OnCloseClick(IInspectable const&, RoutedEventArgs const&)
   {
      Close();
   }

   // ── Actions ──────────────────────────────────────────────────────────────────

   void ConversionWindow::OnBrowseClick(IInspectable const&, RoutedEventArgs const&)
   {
      const std::wstring folder {pickFolder(getHWND())};
      if (!folder.empty()) {
         OutputDirBox().Text(hstring {folder});
      }
   }

   void ConversionWindow::OnCancelClick(IInspectable const&, RoutedEventArgs const&)
   {
      Close();
   }

   // Read the form, persist the choices, run the conversion over every source
   // and report the result.
   void ConversionWindow::OnConvertClick(IInspectable const&, RoutedEventArgs const&)
   {
      const std::wstring extension {SelectedExtension()};
      const std::wstring directory {ResolvedOutputDirectory()};

      if (!IsFormValid(directory)) {
         talay::GUI::showLocalizedMessage(dialogs(), L"ConvertInvalidTitle", L"ConvertInvalidMessage", DialogSeverity::Warning);
         return;
      }

      PersistChoices(extension, directory);
      ReportOutcome(ConvertAll(extension, directory), m_sourcePaths.size());
   }

   // ── Convert pipeline helpers ───────────────────────────────────────────────

   std::wstring ConversionWindow::SelectedExtension()
   {
      auto selected = FormatBox().SelectedItem();
      if (!selected) {
         return L"png";
      }

      auto tag = selected.as<ComboBoxItem>().Tag();
      return tag ? std::wstring {unbox_value<hstring>(tag)} : std::wstring {L"png"};
   }

   std::wstring ConversionWindow::ResolvedOutputDirectory()
   {
      const std::wstring text {OutputDirBox().Text()};
      return text.empty() ? sourceDirectory() : text;
   }

   std::wstring ConversionWindow::sourceDirectory() const
   {
      if (m_sourcePaths.empty()) {
         return {};
      }

      return std::filesystem::path {m_sourcePaths.front()}.parent_path().wstring();
   }

   // A batch needs no name (each file keeps its own stem); a single conversion
   // needs the chosen one.
   bool ConversionWindow::IsFormValid(std::wstring const& directory)
   {
      if (directory.empty() || m_sourcePaths.empty()) {
         return false;
      }

      return isBatch() || !std::wstring {NameBox().Text()}.empty();
   }

   std::wstring ConversionWindow::OutputNameFor(std::wstring const& source)
   {
      if (!isBatch()) {
         return std::wstring {NameBox().Text()};
      }

      return std::filesystem::path {source}.stem().wstring();
   }

   std::size_t ConversionWindow::ConvertAll(std::wstring const& extension, std::wstring const& directory)
   {
      std::size_t converted {0};
      for (std::wstring const& source : m_sourcePaths) {
         if (OImageConverter::getInstance().convert({source, directory, OutputNameFor(source), extension}).success) {
            ++converted;
         }
      }

      return converted;
   }

   void ConversionWindow::PersistChoices(std::wstring const& extension, std::wstring const& directory)
   {
      settings().setConversionFormat(extension);
      settings().setConversionOutputDirectory(directory);
      settings().save();
   }

   // Tell the user what happened: nothing converted is an error, a partial
   // batch is a warning that keeps the dialog open for another try, and a
   // full success acknowledges then closes the dialog.
   void ConversionWindow::ReportOutcome(std::size_t converted, std::size_t total)
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};
      if (converted == 0) {
         dialogs().showMessage({loc.text(L"ErrorTitle"), loc.text(L"ConvertFailedMessage"), DialogSeverity::Error});
         return;
      }

      if (converted < total) {
         dialogs().showMessage({loc.text(L"ConvertDoneTitle"), partialMessage(converted, total), DialogSeverity::Warning});
         return;
      }

      dialogs().showMessage({loc.text(L"ConvertDoneTitle"), doneMessage(total), DialogSeverity::Information},
                            [lifetime = get_strong()] { lifetime->Close(); });
   }

   std::wstring ConversionWindow::doneMessage(std::size_t total) const
   {
      const OLocalizationService& loc {OLocalizationService::getInstance()};
      if (total == 1) {
         return loc.text(L"ConvertDoneMessage");
      }

      return std::vformat(loc.text(L"ConvertBatchDoneMessage"), std::make_wformat_args(total));
   }

   std::wstring ConversionWindow::partialMessage(std::size_t converted, std::size_t total) const
   {
      return std::vformat(OLocalizationService::getInstance().text(L"ConvertBatchPartialMessage"),
                          std::make_wformat_args(converted, total));
   }

   // ── Private helpers ─────────────────────────────────────────────────────────

   talay::Settings::ITalaySettings& ConversionWindow::settings()
   {
      return *m_settings;
   }

   anka::GUI::Dialog::IDialogService& ConversionWindow::dialogs()
   {
      auto& service {ODialogService::getInstance()};
      service.setHost(Content().XamlRoot());
      return service;
   }

   HWND ConversionWindow::getHWND() const
   {
      HWND hwnd {nullptr};
      winrt::check_hresult(this->try_as<::IWindowNative>()->get_WindowHandle(&hwnd));
      return hwnd;
   }

   OverlappedPresenter ConversionWindow::GetPresenter()
   {
      return AppWindow().Presenter().as<OverlappedPresenter>();
   }
}
