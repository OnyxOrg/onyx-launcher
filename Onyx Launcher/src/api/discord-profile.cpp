#include "../includes/api/discord-profile.hpp"
#include "../includes/api/common.hpp"
#include "../includes/api/auth-discord.hpp"
#include "../../deps/httplib.h"
#include "../../deps/json.hpp"

namespace Api
{
	UserInfo GetUserInfo(const std::string& username)
	{
		UserInfo info;
		try
		{
			std::string path = std::string("/api/user/") + username;
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_connection_timeout(2, 0);
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			httplib::Result res = cli.Get(path.c_str());
			if (!res || res->status != 200)
				return info;
			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded()) return info;
			info.ok = true;
			info.username = j.value<std::string>("username", username);
			if (j.contains("discord") && j["discord"].is_object())
			{
				auto d = j["discord"];
				info.discordConnected = d.value("connected", false);
				info.discordId = d.value<std::string>("id", "");
				info.discordUsername = d.value<std::string>("username", "");
				info.discordAvatar = d.value<std::string>("avatar", "");
			}
			info.role = j.value<std::string>("role", "User");
			return info;
		}
		catch (...)
		{
			return info;
		}
	}

	bool UnlinkDiscord(const std::string& username)
	{
		try
		{
			nlohmann::json body; body["username"] = username;
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_connection_timeout(2, 0);
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			httplib::Result res = cli.Post("/api/unlink-discord", body.dump(), "application/json");
			return res && res->status == 200;
		}
		catch (...) { return false; }
	}

	RoleSyncResult SyncRole(const std::string& username, const std::string& discordId)
	{
		RoleSyncResult out;
		try
		{
			nlohmann::json body;
			if (!discordId.empty()) body["discordId"] = discordId;
			else if (!username.empty()) body["username"] = username;
			else return out;
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_connection_timeout(2, 0);
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			httplib::Result res = cli.Post("/api/roles/sync", body.dump(), "application/json");
			if (!res || res->status != 200) return out;
			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded()) return out;
			out.ok = j.contains("role") && j["role"].is_string();
			if (out.ok) out.role = j.value<std::string>("role", "user");
			return out;
		}
		catch (...) { return out; }
	}
}
