#include "auth.hpp"
#include "common.hpp"
#include "../deps/httplib.h"
#include "../deps/json.hpp"

namespace Api
{
	AuthResult Login(const std::string& username, const std::string& password)
	{
		AuthResult result;
		try
		{
			httplib::Client cli(ApiConfig::BaseUrl);
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);

			nlohmann::json body;
			body["username"] = username;
			body["password"] = password;

			auto res = cli.Post("/api/launcher/login", body.dump(), "application/json");
			if (!res)
			{
				result.error = "Connection failed";
				return result;
			}
			if (res->status != 200)
			{
				nlohmann::json errj = nlohmann::json::parse(res->body, nullptr, false);
				if (!errj.is_discarded() && errj.contains("message") && errj["message"].is_string())
					result.error = errj["message"].get<std::string>();
				else
					result.error = "Login failed";
				return result;
			}

			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded())
			{
				result.error = "Bad response";
				return result;
			}

			result.token = j.value<std::string>("token", "");
			result.username = j.value<std::string>("username", username);
			result.role = j.value<std::string>("role", "User");
			result.success = !result.token.empty();
			if (!result.success)
				result.error = "Invalid token";
		}
		catch (...)
		{
			result.error = "Unexpected error";
		}
		return result;
	}
}


