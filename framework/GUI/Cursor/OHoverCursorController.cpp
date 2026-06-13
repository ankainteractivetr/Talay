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
#include "OHoverCursorController.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

#include <winrt/Microsoft.UI.Xaml.Input.h>

#include <optional>
#include <utility>

using namespace anka::GUI::Cursor;
using anka::Core::Object::ObjectIdentity;

using winrt::Windows::Foundation::IInspectable;
using winrt::Microsoft::UI::Xaml::DependencyObject;
using winrt::Microsoft::UI::Xaml::UIElement;
using winrt::Microsoft::UI::Xaml::Input::PointerRoutedEventArgs;

// -----------------------------------------------------------------------------
// OHoverCursorController::Impl Definition
// -----------------------------------------------------------------------------
struct OHoverCursorController::Impl
{
   ObjectIdentity m_identity {L"HoverCursorControllerObject"};

   UIElement                                 m_host;
   std::vector<std::unique_ptr<ICursorRule>> m_rules;
   CursorSink                                m_sink;

   // The element last under the pointer, so a refresh can re-decide in place.
   DependencyObject m_hovered {nullptr};

   UIElement::PointerMoved_revoker  m_moved {};
   UIElement::PointerExited_revoker m_exited {};

   Impl(UIElement host, std::vector<std::unique_ptr<ICursorRule>> rules, CursorSink sink) :
      m_host  {std::move(host)},
      m_rules {std::move(rules)},
      m_sink  {std::move(sink)}
   {
      wire();
   }

   void wire(void)
   {
      m_moved  = m_host.PointerMoved(winrt::auto_revoke, {this, &Impl::onMoved});
      m_exited = m_host.PointerExited(winrt::auto_revoke, {this, &Impl::onExited});
   }

   void onMoved(const IInspectable&, const PointerRoutedEventArgs& args)
   {
      m_hovered = args.OriginalSource().try_as<DependencyObject>();
      apply(resolve(m_hovered));
   }

   // Leaving the host means nothing is hovered; a move back in re-decides.
   void onExited(const IInspectable&, const PointerRoutedEventArgs&)
   {
      m_hovered = nullptr;
      apply(CursorShape::Default);
   }

   // First rule to claim the element wins; an unclaimed element gets the
   // plain arrow (Chain of Responsibility).
   CursorShape resolve(const DependencyObject& hovered) const
   {
      if (!hovered)
         return CursorShape::Default;

      for (const std::unique_ptr<ICursorRule>& rule : m_rules)
         if (const std::optional<CursorShape> shape {rule->resolve(hovered)})
            return *shape;

      return CursorShape::Default;
   }

   void apply(CursorShape shape) const
   {
      if (m_sink)
         m_sink(shape);
   }

   void refresh(void)
   {
      apply(resolve(m_hovered));
   }
};

// -----------------------------------------------------------------------------
// OHoverCursorController Implementation
// -----------------------------------------------------------------------------
OHoverCursorController::OHoverCursorController(UIElement host,
                                               std::vector<std::unique_ptr<ICursorRule>> rules,
                                               CursorSink sink) :
   m_pImpl {std::make_unique<Impl>(std::move(host), std::move(rules), std::move(sink))}
{}
// -----------------------------------------------------------------------------
OHoverCursorController::~OHoverCursorController() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OHoverCursorController::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OHoverCursorController::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OHoverCursorController::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OHoverCursorController::refresh(void)
{
   m_pImpl->refresh();
}
// -----------------------------------------------------------------------------
