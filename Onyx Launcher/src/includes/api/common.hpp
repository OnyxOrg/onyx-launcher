#pragma once

#include <string>
#include <cstdlib>

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
		return GetPrimaryBaseUrl() + std::string("/api/auth/discord/redirect");
	}

	inline std::string BuildDiscordAuthorizeUrl(const std::string& rawState)
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
		url += "&state=" + rawState;
		return url;
	}

	// New helpers for launcher-initiated Discord login
	inline std::string BuildLauncherDiscordLoginState(const std::string& nonce)
	{
		return std::string("discord_oauth:") + nonce;
	}

	inline std::string BuildLauncherPollPath(const std::string& nonce)
	{
		return std::string("/api/launcher/oauth/poll?nonce=") + nonce;
	}

	// Build a Discord CDN avatar URL. If avatarHash is empty, return empty string.
	// size must be a power of two from 16 to 4096. We default to 128.
	inline std::string BuildDiscordAvatarUrl(const std::string& userId, const std::string& avatarHash, int size = 128)
	{
		if (userId.empty() || avatarHash.empty()) return std::string();
		// Animated hashes start with "a_" and should use .gif. Otherwise .png
		bool isGif = avatarHash.rfind("a_", 0) == 0;
		char buf[256] = {0};
		_snprintf_s(buf, _TRUNCATE, "https://cdn.discordapp.com/avatars/%s/%s.%s?size=%d",
			userId.c_str(), avatarHash.c_str(), isGif ? "gif" : "png", size);
		return std::string(buf);
	}
}