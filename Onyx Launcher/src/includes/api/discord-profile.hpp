#pragma once

#include <string>
#include "auth.hpp"

namespace Api
{
	// GET /api/user/:username (from bot API) → returns discord info
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

	// POST /api/roles/sync { discordId? username? } → returns { role }
	struct RoleSyncResult
	{
		bool ok = false;
		std::string role; // lowercase from API, e.g., "user", "staff", ...
	};

	RoleSyncResult SyncRole(const std::string& username, const std::string& discordId);

    // OAuth helpers moved to auth-discord.hpp
}


