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
#include "OSettingsPresenter.hpp"
#include "framework/Core/Object/ObjectIdentity.hpp"

using namespace talay::Settings;
using anka::Core::Object::ObjectIdentity;
using talay::Shell::IShellIntegration;

// -----------------------------------------------------------------------------
// OSettingsPresenter::Impl Definition
// -----------------------------------------------------------------------------
struct OSettingsPresenter::Impl
{
	ObjectIdentity m_identity {L"SettingsPresenterObject"};

	ITalaySettings& m_settings;
	IShellIntegration& m_integration;

	Impl(ITalaySettings& settings, IShellIntegration& integration) :
		m_settings    {settings},
		m_integration {integration}
	{}

	// The ini is the source of truth for every toggle, including file types:
	// it carries the default and the user's saved choice, while the registry is
	// just the effect an apply reconciles to (via IShellIntegration).
	SettingsState currentState(void) const
	{
		return {.language            = m_settings.getLanguage(),
		        .alwaysOnTop         = m_settings.isAlwaysOnTopEnabled(),
		        .autoAddToLibrary    = m_settings.isAutoAddToLibraryEnabled(),
		        .fileTypesRegistered = m_settings.isFileTypesRegistrationEnabled()};
	}

	// The struct's own member initialisers ARE the factory defaults.
	static SettingsState defaultState(void)
	{
		return {};
	}

	// Re-read the file first: another window may have written keys since this
	// process loaded, and save() rewrites the whole file from memory.
	void persist(const SettingsState& state)
	{
		m_settings.load();
		m_settings.setLanguage(state.language);
		m_settings.setAlwaysOnTopEnabled(state.alwaysOnTop);
		m_settings.setAutoAddToLibraryEnabled(state.autoAddToLibrary);
		m_settings.setFileTypesRegistrationEnabled(state.fileTypesRegistered);
		m_settings.save();
	}

	SettingsApplyResult apply(const SettingsState& state)
	{
		const SettingsApplyResult result {
			.languageChanged = state.language != m_settings.getLanguage(),
			.alwaysOnTop     = state.alwaysOnTop};

		persist(state);
		m_integration.applyFileTypes(state.fileTypesRegistered);
		return result;
	}

	// A full factory reset routed through the same seams as an apply: the store
	// resets every key, the registry is reconciled to the default, and the
	// result lets the window run its usual reactions (a saved language clears
	// to "follow the OS", so a restart prompt fires only if one was set).
	SettingsApplyResult resetAllToDefaults(void)
	{
		const SettingsState defaults {defaultState()};
		const SettingsApplyResult result {
			.languageChanged = m_settings.getLanguage().has_value(),
			.alwaysOnTop     = defaults.alwaysOnTop};

		m_settings.resetToDefaults();
		m_integration.applyFileTypes(defaults.fileTypesRegistered);
		return result;
	}
};

// -----------------------------------------------------------------------------
// OSettingsPresenter Implementation
// -----------------------------------------------------------------------------
OSettingsPresenter::OSettingsPresenter(ITalaySettings& settings,
                                       IShellIntegration& integration) :
	m_pImpl {std::make_unique<Impl>(settings, integration)}
{}
// -----------------------------------------------------------------------------
OSettingsPresenter::~OSettingsPresenter() noexcept = default;
// -----------------------------------------------------------------------------
const std::wstring& OSettingsPresenter::getName(void) const
{
	return m_pImpl->m_identity.name();
}
// -----------------------------------------------------------------------------
void OSettingsPresenter::setName(const std::wstring& name)
{
	m_pImpl->m_identity.setName(name);
}
// -----------------------------------------------------------------------------
std::uint64_t OSettingsPresenter::getId(void) const
{
	return m_pImpl->m_identity.id();
}
// -----------------------------------------------------------------------------
SettingsState OSettingsPresenter::currentState(void) const
{
	return m_pImpl->currentState();
}
// -----------------------------------------------------------------------------
SettingsState OSettingsPresenter::defaultState(void) const
{
	return Impl::defaultState();
}
// -----------------------------------------------------------------------------
SettingsApplyResult OSettingsPresenter::apply(const SettingsState& state)
{
	return m_pImpl->apply(state);
}
// -----------------------------------------------------------------------------
SettingsApplyResult OSettingsPresenter::resetAllToDefaults(void)
{
	return m_pImpl->resetAllToDefaults();
}
// -----------------------------------------------------------------------------
