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

#ifndef TALAY_LAUNCH_LAUNCHREQUEST_HPP
#define TALAY_LAUNCH_LAUNCHREQUEST_HPP

#include <string>
#include <string_view>
#include <vector>

namespace talay::Launch
{
   // Why this process was started. Normal shows an empty viewer; OpenFile
   // shows the viewer with a file; Convert shows only the conversion dialog;
   // the shell modes do registry work and exit without UI (installer hooks);
   // Relaunch is the language-restart respawn — it must take over as the new
   // single instance instead of forwarding to the outgoing one that spawned it.
   enum class LaunchMode
   {
      Normal,
      OpenFile,
      Convert,
      RegisterShell,
      UnregisterShell,
      Relaunch
   };

   struct LaunchRequest
   {
      LaunchMode mode {LaunchMode::Normal};
      std::vector<std::wstring> files;
   };

   // The instance channel cooperating Talay processes meet on.
   inline constexpr std::wstring_view k_instanceChannel {L"Talay.Instance"};

   // The argument a restart passes itself so the new process knows to seize the
   // single-instance role rather than forward to its dying predecessor.
   inline constexpr std::wstring_view k_relaunchArg {L"--relaunch"};

   // This process's own command line as a request.
   LaunchRequest parseCommandLine(void);

   // The wire format between instances: the same request, encoded as a
   // newline-separated payload for the instance channel and back.
   std::wstring  encodePayload(const LaunchRequest& request);
   LaunchRequest decodePayload(const std::wstring& payload);
}

#endif // TALAY_LAUNCH_LAUNCHREQUEST_HPP
