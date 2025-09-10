#pragma once

#include "includes/core/app_state.hpp"
#include "includes/core/auth_flow.hpp"
#include "includes/core/overlay.hpp"
#include "includes/core/items/custom.hpp"

// Forward declare Alpha (from main.hpp)
class Alpha;

namespace Views
{
	void RenderLogin(AppState& state, Alpha& alpha, Alpha& subalpha);
}


