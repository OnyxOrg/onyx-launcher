#pragma once

#include "includes/core/app_state.hpp"
#include "includes/core/main.hpp"
#include "includes/core/items/custom.hpp"
#include "includes/api/discord-profile.hpp"

namespace UI
{
	// Renders the unlink confirmation modal. No-ops if state.showUnlinkConfirmationModal is false.
	void RenderUnlinkConfirmationModal(AppState& state);
	
	// Renders the unlink password modal. No-ops if state.showUnlinkPasswordModal is false.
	void RenderUnlinkModal(AppState& state);
}


