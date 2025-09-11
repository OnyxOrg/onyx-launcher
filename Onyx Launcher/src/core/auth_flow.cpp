#include "includes/core/auth_flow.hpp"
#include <thread>

namespace AuthFlow
{
	bool Login(AppState& state, const std::string& username, const std::string& password, std::string& outError)
	{
		Api::AuthResult res = Api::Login(username, password);
		if (!res.success)
		{
			outError = res.error;
			return false;
		}
		state.token = res.token;
		state.username = res.username;
		state.role = "User";
		state.authenticated = true;
		state.ownedProducts = Api::GetUserLibrary(state.username);
		return true;
	}

	void KickoffAsyncLogin(AppState& state, const std::string& username, const std::string& password)
	{
		state.isLoading = true;
		state.loginState = 1;
		state.loginErrMsg[0] = '\0';
		std::thread([&state, username, password]() {
			std::string err;
			bool ok = Login(state, username, password, err);
			if (ok)
			{
				state.loginState = 2;
				// Try to prefetch discord link info (username + id only)
				auto ui = Api::GetUserInfo(state.username);
				if (ui.ok) { state.discordId = ui.discordId; state.discordUsername = ui.discordUsername; }
			}
			else
			{
				state.loginError = err;
				state.loginState = 3;
			}
		}).detach();
	}
}

