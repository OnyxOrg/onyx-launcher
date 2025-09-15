#include "includes/core/functional/auth_flow.hpp"
#include "includes/api/discord-profile.hpp"
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
		state.role = res.role.empty() ? "User" : res.role;
		state.authenticated = true;
		state.ownedProducts = Api::GetUserLibrary(state.username);

		// Immediately sync live Discord roles via bot endpoint to refresh role
		{
			auto sync = Api::SyncRole(state.username, state.discordId);
			if (sync.ok && !sync.role.empty())
				state.role = sync.role;
		}
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
				// Prefetch discord link info (including createdVia)
				auto ui = Api::GetUserInfo(state.username);
				if (ui.ok) { state.discordId = ui.discordId; state.discordUsername = ui.discordUsername; state.createdVia = ui.createdVia; }
			}
			else
			{
				state.loginError = err;
				state.loginState = 3;
			}
		}).detach();
	}
}
