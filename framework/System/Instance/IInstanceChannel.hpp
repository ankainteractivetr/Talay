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

#ifndef ANKA_SYSTEM_INSTANCE_IINSTANCECHANNEL_HPP
#define ANKA_SYSTEM_INSTANCE_IINSTANCECHANNEL_HPP

#include "framework/Core/Object/IObject.hpp"

#include <functional>
#include <string>

namespace anka::System::Instance
{
   // Delivered on the thread that claimed the channel (which therefore must
   // pump messages -- a GUI thread does).
   using ChannelMessageCallback = std::function<void(const std::wstring& payload)>;

   /*
   * Abstract Instance Channel Interface
   *
   * One-owner inter-process mailbox for cooperating instances of the same
   * application. Exactly one process per channel name becomes the owner
   * (claim); every other process can hand it a payload (send) and exit.
   * The classic Windows pattern behind it -- a named mutex electing the
   * owner and WM_COPYDATA carrying the payload -- stays an implementation
   * detail behind this seam.
   *
   */
   class IInstanceChannel : public anka::Core::Object::IObject
   {
      public:

         virtual ~IInstanceChannel() noexcept override = default;

         // Try to become the channel's owner. True: this process now listens
         // and stays the owner until it dies. False: another process owns the
         // channel -- talk to it with send().
         virtual bool claim(const std::wstring& channelName) = 0;

         virtual bool isOwner(void) const = 0;

         // What the owner runs for every payload that arrives.
         virtual void onMessage(ChannelMessageCallback callback) = 0;

         // Deliver a payload to the channel's owner. Retries briefly while the
         // owner is still starting up (several processes spawned at once), so
         // a send right after a failed claim is safe. False when no owner
         // could be reached.
         virtual bool send(const std::wstring& channelName, const std::wstring& payload) = 0;
   };
}

#endif // ANKA_SYSTEM_INSTANCE_IINSTANCECHANNEL_HPP
