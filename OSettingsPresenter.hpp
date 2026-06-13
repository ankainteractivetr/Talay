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

#ifndef TALAY_SETTINGS_OSETTINGSPRESENTER_HPP
#define TALAY_SETTINGS_OSETTINGSPRESENTER_HPP

#include "ISettingsPresenter.hpp"
#include "ITalaySettings.hpp"
#include "IShellIntegration.hpp"

#include <memory>

namespace talay
{
	namespace Settings
	{
		/*
		*
		* Concrete OSettingsPresenter Class
		*
		* Dependency-injected with the window's settings store and the shared
		* shell-integration coordinator; owns the "what changed" bookkeeping on
		* apply and delegates the registry effect of the file-types toggle.
		*
		*/
		class OSettingsPresenter final : public ISettingsPresenter
		{
			public:

				OSettingsPresenter(ITalaySettings& settings,
				                   talay::Shell::IShellIntegration& integration);
				OSettingsPresenter(const OSettingsPresenter& object) = delete;
				OSettingsPresenter(OSettingsPresenter&& object) noexcept = delete;

				~OSettingsPresenter() noexcept override;

				OSettingsPresenter& operator=(const OSettingsPresenter& object) = delete;
				OSettingsPresenter& operator=(OSettingsPresenter&& object) noexcept = delete;

				//
				// IObject overrides
				//

				const std::wstring& getName(void) const override;
				void                setName(const std::wstring& name) override;

				std::uint64_t getId(void) const override;

				//
				// ISettingsPresenter overrides
				//

				SettingsState currentState(void) const override;
				SettingsState defaultState(void) const override;

				SettingsApplyResult apply(const SettingsState& state) override;

				SettingsApplyResult resetAllToDefaults(void) override;


			private:

				struct Impl;
				std::unique_ptr<Impl> m_pImpl;
		};
	}
}

#endif // TALAY_SETTINGS_OSETTINGSPRESENTER_HPP
