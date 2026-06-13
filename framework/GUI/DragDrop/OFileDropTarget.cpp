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
#include "OFileDropTarget.hpp"

// The drop payload lives in the data-transfer projection, which the pch does
// not pull in.
#include <winrt/Windows.ApplicationModel.DataTransfer.h>

#include "framework/Core/Object/ObjectIdentity.hpp"

#include <algorithm>
#include <cwctype>
#include <filesystem>
#include <unordered_set>
#include <utility>

using namespace anka::Core::Object;
using namespace anka::GUI::DragDrop;

using winrt::Windows::Foundation::IInspectable;
using winrt::Windows::ApplicationModel::DataTransfer::DataPackageOperation;
using winrt::Windows::ApplicationModel::DataTransfer::DataPackageView;
using winrt::Windows::ApplicationModel::DataTransfer::StandardDataFormats;
using winrt::Windows::Storage::IStorageItem;
using winrt::Windows::Storage::StorageItemTypes;
using winrt::Microsoft::UI::Xaml::DragEventArgs;
using winrt::Microsoft::UI::Xaml::UIElement;

namespace
{
   std::wstring toUpper(std::wstring text)
   {
      std::transform(text.begin(), text.end(), text.begin(),
                     [](wchar_t character) { return static_cast<wchar_t>(std::towupper(character)); });
      return text;
   }

   // The allowed extensions, upper-cased into a set so the per-file test is
   // O(1) and case-insensitive (the shell mixes ".JPG" and ".jpg" freely).
   std::unordered_set<std::wstring> buildExtensionSet(const std::vector<std::wstring>& extensions)
   {
      std::unordered_set<std::wstring> set;
      for (const std::wstring& extension : extensions)
         set.insert(toUpper(extension));

      return set;
   }

   bool passesFilter(const IStorageItem& item, const std::unordered_set<std::wstring>& allowed)
   {
      if (!item.IsOfType(StorageItemTypes::File))
         return false;

      const std::wstring extension {std::filesystem::path {std::wstring {item.Path()}}.extension().wstring()};
      return allowed.empty() || allowed.contains(toUpper(extension));
   }

   // Deliver the first file of the payload that passes the filter. A free
   // coroutine on purpose: it owns copies of everything it touches, so the
   // drop target may die while the (asynchronous) storage-item query is in
   // flight without leaving a dangling 'this' behind the suspension point.
   winrt::fire_and_forget deliverDrop(DataPackageView payload,
                                      std::unordered_set<std::wstring> allowed,
                                      FileDroppedCallback deliver)
   {
      const auto items {co_await payload.GetStorageItemsAsync()};

      for (const IStorageItem& item : items) {
         if (passesFilter(item, allowed)) {
            deliver(std::wstring {item.Path()});
            co_return;
         }
      }
   }
}

// -----------------------------------------------------------------------------
// OFileDropTarget::Impl Definition
// -----------------------------------------------------------------------------
struct OFileDropTarget::Impl
{
   ObjectIdentity m_identity {L"FileDropTargetObject"};

   std::unordered_set<std::wstring> m_allowed;
   FileDroppedCallback              m_onDrop;

   UIElement m_surface {nullptr};

   UIElement::DragOver_revoker m_dragOver {};
   UIElement::Drop_revoker     m_drop {};

   void attach(const UIElement& dropSurface)
   {
      m_surface = dropSurface;
      m_surface.AllowDrop(true);
      m_dragOver = m_surface.DragOver(winrt::auto_revoke, {this, &Impl::onDragOver});
      m_drop     = m_surface.Drop(winrt::auto_revoke, {this, &Impl::onDrop});
   }

   // Offer a Copy for any drag carrying storage items; the per-file filter
   // has to wait for the drop, because the payload's paths are only readable
   // asynchronously while the drag-over answer must be synchronous.
   void onDragOver(const IInspectable&, const DragEventArgs& args)
   {
      const bool hasFiles {args.DataView().Contains(StandardDataFormats::StorageItems())};

      args.AcceptedOperation(hasFiles ? DataPackageOperation::Copy : DataPackageOperation::None);
      args.Handled(true);
   }

   void onDrop(const IInspectable&, const DragEventArgs& args)
   {
      if (m_onDrop && args.DataView().Contains(StandardDataFormats::StorageItems()))
         deliverDrop(args.DataView(), m_allowed, m_onDrop);
   }
};

// -----------------------------------------------------------------------------
// OFileDropTarget Implementation
// -----------------------------------------------------------------------------
OFileDropTarget::OFileDropTarget() :
   m_pImpl {std::make_unique<Impl>()}
{}
// -----------------------------------------------------------------------------
OFileDropTarget::~OFileDropTarget() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OFileDropTarget::getName(void) const
{
   return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OFileDropTarget::setName(const std::wstring& name)
{
   m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OFileDropTarget::getId(void) const
{
   return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
void OFileDropTarget::attach(const UIElement& dropSurface)
{
   m_pImpl->attach(dropSurface);
}
// -----------------------------------------------------------------------------
void OFileDropTarget::allowExtensions(const std::vector<std::wstring>& extensions)
{
   m_pImpl->m_allowed = buildExtensionSet(extensions);
}
// -----------------------------------------------------------------------------
void OFileDropTarget::onFileDropped(FileDroppedCallback callback)
{
   m_pImpl->m_onDrop = std::move(callback);
}
// -----------------------------------------------------------------------------
