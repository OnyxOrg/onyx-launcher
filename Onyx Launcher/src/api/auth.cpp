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
		return result;
	}
}


