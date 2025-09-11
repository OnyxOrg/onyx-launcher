#pragma once

#include <string>

namespace Api
{
	struct AuthResult
	{
		bool success = false;
		std::string token;
		std::string username;
		std::string role;
		std::string error;
	};

	// Performs POST /api/launcher/login
	AuthResult Login(const std::string& username, const std::string& password);

	// Performs POST /api/webapp/register
	AuthResult Register(const std::string& username, const std::string& password, const std::string& licenseKey);

	// GET /api/user/:username (from webapp) → returns discord info
	struct UserInfo
	{
		bool ok = false;
		std::string username;     // app username
		std::string discordId;
		std::string discordUsername;
		std::string discordAvatar; // hash only
		bool discordConnected = false;
		std::string role;
	};

	UserInfo GetUserInfo(const std::string& username);

	// POST /api/unlink-discord { username } → returns success
	bool UnlinkDiscord(const std::string& username);
}


