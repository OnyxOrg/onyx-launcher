#include "../includes/api/auth.hpp"
#include "../includes/api/common.hpp"
#include "../includes/api/discord-profile.hpp"
#include "../../deps/httplib.h"
#include "../../deps/json.hpp"

namespace Api
{
	AuthResult Login(const std::string& username, const std::string& password)
	{
		AuthResult result;
		try
		{
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_connection_timeout(2, 0);
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
			return result;
		}
		catch (...)
		{
			result.error = "Unexpected error";
			return result;
		}
	}

	AuthResult Register(const std::string& username, const std::string& password, const std::string& licenseKey)
	{
		AuthResult out;
		try
		{
			nlohmann::json body;
			body["username"] = username;
			body["password"] = password;
			body["key"] = licenseKey;

			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_connection_timeout(2, 0);
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);
			httplib::Result res = cli.Post("/api/webapp/register", body.dump(), "application/json");
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
}
