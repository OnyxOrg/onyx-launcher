#include "library.hpp"
#include "common.hpp"
#include "../deps/httplib.h"
#include "../deps/json.hpp"

namespace Api
{
	static std::string MapStatusToString(const std::string& s)
	{
		// passthrough, but normalize to lowercase
		std::string out = s;
		for (char& c : out) c = (char)tolower((unsigned char)c);
		return out;
	}

	std::vector<LibraryProduct> GetUserLibrary(const std::string& username)
	{
		std::vector<LibraryProduct> result;
		try
		{
			// Use bot API base URL so webapp doesn't need to be online
			httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
			cli.set_read_timeout(5, 0);
			cli.set_write_timeout(5, 0);

			// Bot API: /api/webapp/library/:username -> [{ name, duration, expiresAt }]
			std::string path = "/api/webapp/library/" + username;
			auto res = cli.Get(path.c_str());
			if (!res || res->status != 200)
				return result;

			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded() || !j.is_array())
				return result;

			for (const auto& it : j)
			{
				LibraryProduct p;
				p.name = it.value("name", std::string());
				p.status = std::string("online");
				p.durationLabel = it.value("duration", std::string());
				if (it.contains("expiresAt") && !it["expiresAt"].is_null())
					p.expiresAt = it["expiresAt"].get<std::string>();
				else
					p.expiresAt.clear();
				result.push_back(std::move(p));
			}
		}
		catch (...)
		{
			// ignore, return empty
		}
		return result;
	}
}


