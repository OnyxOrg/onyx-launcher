#include "../includes/api/auth.hpp"
#include "../includes/api/common.hpp"
#include "../../deps/httplib.h"
#include "../../deps/json.hpp"

namespace Api
{
	static bool TryLogin(const std::string& baseUrl, const std::string& username, const std::string& password, AuthResult& out)
	{
		try
		{
			httplib::Client cli(baseUrl.c_str());
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);

			nlohmann::json body;
			body["username"] = username;
			body["password"] = password;

			// Always use Discord bot API login path
			const char* path = "/api/webapp/login";
			auto res = cli.Post(path, body.dump(), "application/json");
			if (!res)
			{
				out.error = "Connection failed";
				return false;
			}
			if (res->status != 200)
			{
				nlohmann::json errj = nlohmann::json::parse(res->body, nullptr, false);
				if (!errj.is_discarded() && errj.contains("message") && errj["message"].is_string())
					out.error = errj["message"].get<std::string>();
				else
					out.error = "Login failed";
				return false;
			}

			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded())
			{
				out.error = "Bad response";
				return false;
			}

			out.token = j.value<std::string>("token", "");
			out.username = j.value<std::string>("username", username);
			out.role = j.value<std::string>("role", "User");
			out.success = !out.token.empty();
			if (!out.success)
				out.error = "Invalid token";
			return out.success;
		}
		catch (...)
		{
			out.error = "Unexpected error";
			return false;
		}
	}

	AuthResult Login(const std::string& username, const std::string& password)
	{
		AuthResult result;
		if (TryLogin(ApiConfig::GetPrimaryBaseUrl(), username, password, result))
			return result;
		// Fallbacks for local/dev environments
		if (TryLogin("http://localhost:3000", username, password, result))
			return result;
		if (TryLogin("http://127.0.0.1:3000", username, password, result))
			return result;
		return result;
	}

	AuthResult Register(const std::string& username, const std::string& password, const std::string& licenseKey)
	{
		AuthResult out;
		try
		{
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);

			nlohmann::json body;
			body["username"] = username;
			body["password"] = password;
			body["key"] = licenseKey;

			auto res = cli.Post("/api/webapp/register", body.dump(), "application/json");
			if (!res)
			{
				out.error = "Connection failed";
				return out;
			}
			if (res->status != 201)
			{
				nlohmann::json errj = nlohmann::json::parse(res->body, nullptr, false);
				if (!errj.is_discarded() && errj.contains("message") && errj["message"].is_string())
					out.error = errj["message"].get<std::string>();
				else
					out.error = "Registration failed";
				return out;
			}
			out.success = true;
			out.username = username;
			return out;
		}
		catch (...)
		{
			out.error = "Unexpected error";
			return out;
		}
	}

	UserInfo GetUserInfo(const std::string& username)
	{
		UserInfo info;
		try
		{
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			std::string path = std::string("/api/user/") + username;
			auto res = cli.Get(path.c_str());
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
				// avatar loading removed for now
				info.discordAvatar = "";
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
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			nlohmann::json body; body["username"] = username;
			auto res = cli.Post("/api/unlink-discord", body.dump(), "application/json");
			return res && res->status == 200;
		}
		catch (...) { return false; }
	}

	RoleSyncResult SyncRole(const std::string& username, const std::string& discordId)
	{
		RoleSyncResult out;
		try
		{
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			nlohmann::json body;
			if (!discordId.empty()) body["discordId"] = discordId;
			else if (!username.empty()) body["username"] = username;
			else return out;
			auto res = cli.Post("/api/roles/sync", body.dump(), "application/json");
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
