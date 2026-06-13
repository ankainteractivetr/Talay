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

#ifndef ANKA_SYSTEM_INSTANCE_OINSTANCECHANNEL_HPP
#define ANKA_SYSTEM_INSTANCE_OINSTANCECHANNEL_HPP

#include "IInstanceChannel.hpp"

#include <memory>

namespace anka::System::Instance
{
   /*
   *
   * Concrete OInstanceChannel Class
   *
   * Named mutex (Local\ session namespace) elects the owner; a hidden
   * message-only window receives WM_COPYDATA payloads. Pure Windows API.
   *
   */
   class OInstanceChannel final : public IInstanceChannel
   {
      public:

         OInstanceChannel();
         explicit OInstanceChannel(const std::wstring& objectName);
         OInstanceChannel(const OInstanceChannel& object) = delete;
         OInstanceChannel(OInstanceChannel&& object) noexcept = delete;

         ~OInstanceChannel() noexcept override;

         OInstanceChannel& operator=(const OInstanceChannel& object) = delete;
         OInstanceChannel& operator=(OInstanceChannel&& object) noexcept = delete;

         //
         // IObject overrides
         //

         const std::wstring& getName(void) const override;
         void                setName(const std::wstring& name) override;

         std::uint64_t getId(void) const override;

         //
         // IInstanceChannel overrides
         //

         bool claim(const std::wstring& channelName) override;

         bool isOwner(void) const override;

         void onMessage(ChannelMessageCallback callback) override;

         bool send(const std::wstring& channelName, const std::wstring& payload) override;


      private:

         struct Impl;
         std::unique_ptr<Impl> m_pImpl;
   };
}

#endif // ANKA_SYSTEM_INSTANCE_OINSTANCECHANNEL_HPP
