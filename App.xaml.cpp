#include "pch.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"
#include "ConversionWindow.xaml.h"
#include "ImageFormats.hpp"
#include "LaunchRequest.hpp"
#include "OTalaySettings.hpp"
#include "Language.hpp"
#include "OShellIntegration.hpp"
#include "framework/Core/Localization/OLocalizationService.hpp"
#include "framework/GUI/Dialog/ODialogService.hpp"
#include "framework/System/Instance/OInstanceChannel.hpp"
#include "framework/System/Shell/OShellRegistrar.hpp"
#include "framework/System/Process/ExecutablePath.hpp"

#include <optional>
#include <string>

using namespace winrt;
using namespace Microsoft::UI::Xaml;

using talay::Launch::LaunchMode;
using talay::Launch::LaunchRequest;

// To learn more about WinUI, the WinUI project structure,
// and more about our project templates, see: http://aka.ms/winui-project-info.

namespace
{
   // The saved UI-language choice next to the executable as a BCP-47 tag, or empty
   // when none is set (then the app opens in the OS display language). The app owns
   // the language set, so it converts its choice to the tag the service expects.
   std::optional<std::wstring> savedLanguageTag()
   {
      talay::Settings::OTalaySettings settings {
         L"OTalaySettings", anka::System::Process::executableDirectory()};
      settings.load();

      const std::optional<talay::Localization::Language> language {settings.getLanguage()};
      if (!language)
         return std::nullopt;

      return std::wstring {talay::Localization::toBcp47(*language)};
   }

   // Hand the framework dialog service Talay's chrome — the app ContentDialog style
   // and the button captions in the resolved language — so its dialogs match the
   // rest of the UI without the framework knowing any Talay specifics.
   void configureDialogs()
   {
      const auto& loc {anka::Core::Localization::OLocalizationService::getInstance()};
      anka::GUI::Dialog::ODialogService::getInstance().configure({
         .styleKey   = L"TalayContentDialogStyle",
         .okCaption  = loc.text(L"CommonOk"),
         .yesCaption = loc.text(L"CommonYes"),
         .noCaption  = loc.text(L"CommonNo")});
   }

   std::wstring channelName()
   {
      return std::wstring {talay::Launch::k_instanceChannel};
   }

   // A fresh coordinator bound to the process-wide registrar. Cheap to build
   // (it only wraps the reference), so each shell-maintenance branch makes its
   // own rather than threading one through the launch flow.
   std::unique_ptr<talay::Shell::IShellIntegration> shellIntegration()
   {
      return std::make_unique<talay::Shell::OShellIntegration>(
         anka::System::Shell::OShellRegistrar::getInstance());
   }

   // A restart spawns its successor while still alive, so the successor cannot
   // claim the channel until this process exits and frees the mutex. Poll for it
   // — bounded so a predecessor that refuses to die can never hang the restart.
   constexpr int   k_relaunchClaimAttempts {60};
   constexpr DWORD k_relaunchClaimDelayMs  {50};   // ~3 s ceiling

   // A conversion-only owner can absorb more convert files; any other
   // forwarded launch just pulls the dialog forward (there is no viewer here).
   void deliverToConversion(winrt::Talay::implementation::ConversionWindow& conversion,
                            const LaunchRequest& request)
   {
      if (request.mode == LaunchMode::Convert && !request.files.empty()) {
         conversion.appendSources(request.files);
         return;
      }

      conversion.Activate();
   }
}

namespace winrt::Talay::implementation
{
    /// <summary>
    /// Initializes the singleton application object.  This is the first line of authored code
    /// executed, and as such is the logical equivalent of main() or WinMain().
    /// </summary>
    App::App()
    {
        // Xaml objects should not call InitializeComponent during construction.
        // See https://github.com/microsoft/cppwinrt/tree/master/nuget#initializecomponent

#if defined _DEBUG && !defined DISABLE_XAML_GENERATED_BREAK_ON_UNHANDLED_EXCEPTION
        UnhandledException([](IInspectable const&, UnhandledExceptionEventArgs const& e)
        {
            if (IsDebuggerPresent())
            {
                auto errorMessage = e.Message();
                __debugbreak();
            }
        });
#endif
    }

    // Out-of-line so the unique_ptr member deletes a complete IInstanceChannel.
    App::~App() = default;

    /// <summary>
    /// Invoked when the application is launched.
    /// </summary>
    /// <param name="e">Details about the launch request and process.</param>
    void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        // Resolve the display language before any UI loads, so the x:Uid layer and
        // code-behind lookups resolve against the same language. A saved choice
        // (settings) wins and is forced onto the resource system; otherwise the OS
        // display language is used.
        anka::Core::Localization::OLocalizationService::getInstance().initialize(savedLanguageTag());

        const LaunchRequest request {talay::Launch::parseCommandLine()};
        if (HandleShellMaintenance(request)) {
            Exit();
            return;
        }

        if (ClaimOrForward(request))
            return;

        configureDialogs();

        if (request.mode == LaunchMode::Convert) {
            ShowConversionWindow(request.files);
            return;
        }

        LaunchMainWindow(request);
    }

    // ── Launch-mode branches ──────────────────────────────────────────────────

    // The installer hooks: do the registry work and report "handled" so the app
    // exits without showing any UI.
    bool App::HandleShellMaintenance(LaunchRequest const& request)
    {
        if (request.mode == LaunchMode::RegisterShell)
            return RegisterShellIntegration();

        if (request.mode == LaunchMode::UnregisterShell)
            return UnregisterShellIntegration();

        return false;
    }

    // Register the supported image types under "Open with" and add the
    // "Convert with Talay" verb in one step, then report "handled" so the app
    // exits without UI. Runs per user (HKCU) via the installer's hook.
    bool App::RegisterShellIntegration()
    {
        shellIntegration()->install();
        return true;
    }

    // Uninstall cleans every integration: the per-user verb, the machine-wide
    // verb older installers wrote, and the "Open with" entry.
    bool App::UnregisterShellIntegration()
    {
        shellIntegration()->uninstall();
        return true;
    }

    // The single-instance gate. The first process claims the channel and
    // serves all UI; every later launch — double-clicked exe, file open,
    // Explorer's per-file convert spawns — hands its request to the owner and
    // exits. An unreachable owner (it is just shutting down, e.g. the language
    // restart) makes us inherit the channel and serve the launch ourselves.
    bool App::ClaimOrForward(LaunchRequest const& request)
    {
        m_channel = std::make_unique<anka::System::Instance::OInstanceChannel>(L"InstanceChannelObject");
        if (m_channel->claim(channelName()))
            return false;

        // A restart must become the new owner, never forward to the outgoing
        // instance that spawned it (that would just hand control to a dying
        // process and leave nothing running).
        if (request.mode == LaunchMode::Relaunch) {
            ClaimWhenFree();
            return false;
        }

        if (m_channel->send(channelName(), talay::Launch::encodePayload(request))) {
            Exit();
            return true;
        }

        m_channel->claim(channelName());
        return false;
    }

    // Retry the claim while the predecessor still holds the mutex; it exits
    // moments after spawning us, so this resolves in well under a second.
    void App::ClaimWhenFree()
    {
        for (int attempt {0}; attempt < k_relaunchClaimAttempts; ++attempt) {
            if (m_channel->claim(channelName()))
                return;

            Sleep(k_relaunchClaimDelayMs);
        }
    }

    // The "Convert with Talay" verb is per-user and self-healing: every normal
    // start makes sure it exists and points at THIS executable (dev builds and
    // moved installs included). The file-type associations are the installer's
    // and the settings dialog's to own, so they are left untouched here.
    void App::EnsureShellIntegration()
    {
        shellIntegration()->ensureVerb();
    }

    void App::ShowConversionWindow(std::vector<std::wstring> const& files)
    {
        auto conversion {winrt::make_self<ConversionWindow>()};
        conversion->configure(files, nullptr, talay::Formats::supportedImageFormats());

        m_channel->onMessage([weak = conversion->get_weak()](std::wstring const& payload) {
            if (auto strong {weak.get()})
                deliverToConversion(*strong, talay::Launch::decodePayload(payload));
        });

        window = conversion.as<Window>();
        window.Activate();
    }

    void App::LaunchMainWindow(LaunchRequest const& request)
    {
        EnsureShellIntegration();

        auto main {winrt::make_self<MainWindow>()};
        ListenForInstances(main);

        window = main.as<Window>();
        window.Activate();

        if (request.mode == LaunchMode::OpenFile && !request.files.empty())
            main->openImageFile(request.files.front());
    }

    // Forwarded launches land here: convert requests open the conversion
    // dialog over this window, open requests pull it to the front (and show
    // the file). The channel was already claimed by the gate.
    void App::ListenForInstances(winrt::com_ptr<MainWindow> const& main)
    {
        m_channel->onMessage([weak = main->get_weak()](std::wstring const& payload) {
            if (auto strong {weak.get()})
                strong->onInstanceMessage(payload);
        });
    }
}
