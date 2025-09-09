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

	static std::map<std::string, std::string> TryFetchStatusesFromWebAppInternal(const std::string& username)
	{
		std::map<std::string, std::string> result;
		try
		{
			httplib::Client cli(ApiConfig::GetWebAppBaseUrl().c_str());
			cli.set_read_timeout(2, 0);
			cli.set_write_timeout(2, 0);
			std::string path = "/api/library/" + username;
			auto res = cli.Get(path.c_str());
			if (!res || res->status != 200) return result;
			nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
			if (j.is_discarded() || !j.is_array()) return result;
			for (const auto& it : j)
			{
				std::string name = MapStatusToString(it.value("name", std::string()));
				std::string status = MapStatusToString(it.value("status", std::string("online")));
				if (!name.empty()) result[name] = status;
			}
		}
		catch (...) {}
		return result;
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

			// Enrich with statuses from webapp if available
			auto statusMap = TryFetchStatusesFromWebAppInternal(username);

			for (const auto& it : j)
			{
				LibraryProduct p;
				p.name = it.value("name", std::string());
				{
					std::string lower = MapStatusToString(p.name);
					auto itst = statusMap.find(lower);
					p.status = (itst != statusMap.end()) ? itst->second : std::string("online");
				}
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


