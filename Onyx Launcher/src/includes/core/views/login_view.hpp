#pragma once

#include "includes/core/app_state.hpp"
#include "includes/core/functional/auth_flow.hpp"
#include "includes/core/items/overlay.hpp"
#include "includes/core/items/custom.hpp"
#include "includes/core/utils/credentials.hpp"

// Forward declare Alpha (from main.hpp)
class Alpha;

namespace Views
{
	void RenderLogin(AppState& state, Alpha& alpha, Alpha& subalpha);
}
