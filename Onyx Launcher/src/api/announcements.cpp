#include "../includes/api/announcements.hpp"
#include "../includes/api/common.hpp"
#include "../../deps/httplib.h"
#include "../../deps/json.hpp"

namespace Api
{
    std::vector<Announcement> GetAnnouncements()
    {
        std::vector<Announcement> result;
        try
        {
            // Use bot API base URL
            httplib::Client cli(ApiConfig::GetPrimaryBaseUrl().c_str());
            cli.set_read_timeout(5, 0);
            cli.set_write_timeout(5, 0);

            // Bot API: /api/announcements -> [{ title, content, date }]
            std::string path = "/api/announcements";
            auto res = cli.Get(path.c_str());
            if (!res || res->status != 200)
                return result;

            nlohmann::json j = nlohmann::json::parse(res->body, nullptr, false);
            if (j.is_discarded() || !j.is_array())
                return result;

            for (const auto& it : j)
            {
                Announcement announcement;
                announcement.title = it.value("title", std::string());
                announcement.content = it.value("content", std::string());
                announcement.date = it.value("date", std::string());
                result.push_back(std::move(announcement));
            }
        }
        catch (...)
        {
            // ignore, return empty
        }
        return result;
    }
}
