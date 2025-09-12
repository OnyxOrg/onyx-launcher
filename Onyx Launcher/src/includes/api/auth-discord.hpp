#pragma once

#include <string>
#include "auth.hpp"

namespace Api
{
	// One-shot poll for a launcher token after Discord OAuth
	AuthResult PollDiscordLoginOnce(const std::string& nonce);

	// Starts the Discord login flow: returns nonce and the URL to open
	bool BeginDiscordLoginFlow(const std::string& currentUsername, std::string& outNonce, std::string& outAuthUrl);
}


