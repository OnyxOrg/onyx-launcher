#pragma once

#include "app_state.hpp"
#include "items/overlay.hpp"
#include "time_util.hpp"
#include "items/custom.hpp"
#include "utils/credentials.hpp"

namespace App
{
	// One frame of the application UI
	void RenderFrame(AppState& state);
}