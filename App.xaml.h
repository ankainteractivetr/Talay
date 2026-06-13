#pragma once

#include "App.xaml.g.h"
#include "LaunchRequest.hpp"

#include <memory>
#include <string>
#include <vector>

namespace anka::System::Instance { class IInstanceChannel; }

namespace winrt::Talay::implementation
{
    struct MainWindow;

    struct App : AppT<App>
    {
        App();
        ~App();

        void OnLaunched(Microsoft::UI::Xaml::LaunchActivatedEventArgs const&);

    private:

        // Launch-mode branches (one per LaunchMode family): shell maintenance
        // exits without UI, the single-instance gate forwards later launches
        // to the running process, convert shows only the conversion dialog,
        // and the normal path shows the viewer (optionally opening a file).
        bool HandleShellMaintenance(talay::Launch::LaunchRequest const& request);
        bool RegisterShellIntegration();
        bool UnregisterShellIntegration();
        bool ClaimOrForward(talay::Launch::LaunchRequest const& request);
        void ClaimWhenFree();
        void EnsureShellIntegration();
        void ShowConversionWindow(std::vector<std::wstring> const& files);
        void LaunchMainWindow(talay::Launch::LaunchRequest const& request);
        void ListenForInstances(winrt::com_ptr<MainWindow> const& main);

        winrt::Microsoft::UI::Xaml::Window window{ nullptr };

        // The inter-instance mailbox: the first instance claims it and listens;
        // later "Convert with Talay" processes hand their files over through it.
        std::unique_ptr<anka::System::Instance::IInstanceChannel> m_channel;
    };
}
