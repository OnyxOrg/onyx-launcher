#pragma once

#include <string>
#include "auth.hpp"

namespace Api
{
	// One-shot poll for a launcher token after Google OAuth
	AuthResult PollGoogleLoginOnce(const std::string& nonce);

	// Starts the Google login flow: returns nonce and the URL to open
	bool BeginGoogleLoginFlow(const std::string& currentUsername, std::string& outNonce, std::string& outAuthUrl);
}
