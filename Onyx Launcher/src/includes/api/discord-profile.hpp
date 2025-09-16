#pragma once

#include <string>
#include "auth.hpp"

namespace Api
{
	// GET /api/user/:username (from bot API) → returns discord and google info
	struct UserInfo
	{
		bool ok = false;
		std::string username;     // app username
		std::string discordId;
		std::string discordUsername;
		std::string discordAvatar; // hash only
		bool discordConnected = false;
		std::string googleId;
		std::string googleUsername;
		std::string googlePicture; // picture ID for avatar
		bool googleConnected = false;
		std::string role;
		std::string createdVia;
	};

	UserInfo GetUserInfo(const std::string& username);

	// POST /api/unlink-discord { username } → returns success
	bool UnlinkDiscord(const std::string& username, const std::string& newPassword = std::string());

	// POST /api/roles/sync { discordId? username? } → returns { role }
	struct RoleSyncResult
	{
		bool ok = false;
		std::string role; // lowercase from API, e.g., "user", "staff", ...
	};

	RoleSyncResult SyncRole(const std::string& username, const std::string& discordId);

    // OAuth helpers moved to auth-discord.hpp
}


