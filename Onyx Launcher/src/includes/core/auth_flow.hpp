#pragma once

#include "app_state.hpp"

namespace AuthFlow
{
	bool Login(AppState& state, const std::string& username, const std::string& password, std::string& outError);
	void KickoffAsyncLogin(AppState& state, const std::string& username, const std::string& password);
}


