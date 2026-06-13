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

#include "pch.h"
#include "framework/Core/Object/ObjectIdentity.hpp"
#include "OMenuFactory.hpp"

using namespace anka::Core::Object;
using namespace anka::GUI::Menu;
using namespace winrt;
using namespace winrt::Microsoft::UI::Xaml;
using namespace winrt::Microsoft::UI::Xaml::Controls;
using namespace winrt::Microsoft::UI::Xaml::Media;

// -----------------------------------------------------------------------------
// Item construction helpers: each command becomes one flyout item (separator
// descriptors a divider); the item look (icon font, theme colours) follows
// the host's flyout resources.
// -----------------------------------------------------------------------------
namespace
{
   constexpr wchar_t k_iconFont[] {L"Segoe MDL2 Assets"};

   FontIcon makeItemIcon(const std::wstring& glyph)
   {
      FontIcon icon;
      icon.Glyph(hstring {glyph});
      icon.FontFamily(FontFamily {k_iconFont});
      return icon;
   }

   MenuFlyoutItem makeItem(const MenuCommand& command)
   {
      MenuFlyoutItem item;
      item.Text(hstring {command.label});
      if (!command.glyph.empty())
         item.Icon(makeItemIcon(command.glyph));

      item.Click([action = command.action](auto&&, auto&&) {
         if (action)
            action();
      });
      return item;
   }

   MenuFlyoutItemBase makeEntry(const MenuCommand& command)
   {
      if (command.kind == MenuItemKind::Separator)
         return MenuFlyoutSeparator {};

      return makeItem(command);
   }
}

// -----------------------------------------------------------------------------
// OMenuFactory::Impl Definition
// -----------------------------------------------------------------------------
struct OMenuFactory::Impl
{
   ObjectIdentity m_identity {L"MenuFactoryObject"};
};

// -----------------------------------------------------------------------------
// OMenuFactory Implementation
// -----------------------------------------------------------------------------
OMenuFactory& OMenuFactory::getInstance()
{
   static OMenuFactory s_instance;
   return s_instance;
}
// -----------------------------------------------------------------------------
OMenuFactory::OMenuFactory() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OMenuFactory::~OMenuFactory() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OMenuFactory::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OMenuFactory::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OMenuFactory::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
MenuFlyout OMenuFactory::createFlyout(std::span<const MenuCommand> commands) const
{
   MenuFlyout flyout;
   for (const auto& command : commands)
      flyout.Items().Append(makeEntry(command));

   return flyout;
}
// -----------------------------------------------------------------------------
void OMenuFactory::attachContextMenu(const UIElement& target,
                                     std::span<const MenuCommand> commands) const
{
   target.ContextFlyout(createFlyout(commands));
}
// -----------------------------------------------------------------------------
