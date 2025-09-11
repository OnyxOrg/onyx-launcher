#pragma once

#include <string>

namespace ApiConfig
{
	// Primary API host (Discord bot API)
	inline const char* DefaultPrimary = "http://localhost:3000";


	inline std::string GetPrimaryBaseUrl()
	{
		return std::string(DefaultPrimary);
	}


	// Discord OAuth configuration (should match the webapp configuration)
	inline const char* DiscordClientId = "1329500067492528248"; // TODO: set real client id

	inline std::string GetDiscordRedirectUri()
	{
		// Use the Discord bot API for OAuth redirect (no webapp required)
		return GetPrimaryBaseUrl() + std::string("/api/auth/discord/redirect");
	}

	inline std::string BuildDiscordAuthorizeUrl(const std::string& stateUsername)
	{
		auto PercentEncode = [](const std::string& s) {
			std::string out; out.reserve(s.size() * 3);
			const char* hex = "0123456789ABCDEF";
			for (unsigned char c : s) {
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
					out.push_back((char)c);
				} else {
					out.push_back('%'); out.push_back(hex[(c >> 4) & 0xF]); out.push_back(hex[c & 0xF]);
				}
			}
			return out;
		};

		std::string url = "https://discord.com/oauth2/authorize";
		url += "?client_id=" + std::string(DiscordClientId);
		url += "&response_type=code";
		url += "&redirect_uri=" + PercentEncode(GetDiscordRedirectUri());
		url += "&scope=identify%20email%20connections%20guilds%20guilds.join";
		url += "&state=" + stateUsername;
		return url;
	}
}