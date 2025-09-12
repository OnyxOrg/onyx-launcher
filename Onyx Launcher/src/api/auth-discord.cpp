#include "../includes/api/auth-discord.hpp"
#include "../includes/api/common.hpp"
#include "../../deps/httplib.h"
#include "../../deps/json.hpp"

namespace Api
{
	static std::string GenerateNonce()
	{
		static const char* alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
		std::string out; out.resize(24);
		for (size_t i = 0; i < out.size(); ++i)
			out[i] = alphabet[rand() % 62];
		return out;
	}

	AuthResult PollDiscordLoginOnce(const std::string& nonce)
	{
		AuthResult out;
		try
		{
			std::string path = ApiConfig::BuildLauncherPollPath(nonce);
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_connection_timeout(2, 0);
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			httplib::Result res = cli.Get(path.c_str());
			if (!res) { out.error = "Connection failed"; return out; }
			if (res->status == 204) { out.error = "pending"; return out; }
			if (res->status != 200) { out.error = "poll failed"; return out; }
			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded()) { out.error = "bad json"; return out; }
			out.token = j.value<std::string>("token", "");
			out.username = j.value<std::string>("username", "");
			out.role = j.value<std::string>("role", "user");
			out.success = !out.token.empty();
			if (!out.success) out.error = "no token";
			return out;
		}
		catch (...) { out.error = "error"; return out; }
	}

	bool BeginDiscordLoginFlow(const std::string& /*currentUsername*/, std::string& outNonce, std::string& outAuthUrl)
	{
		outNonce = GenerateNonce();
		std::string state = ApiConfig::BuildLauncherDiscordLoginState(outNonce);
		outAuthUrl = ApiConfig::GetPrimaryBaseUrl() + std::string("/api/launcher/oauth/authorize?state=") + state;
		return true;
	}
}


