#pragma once
#include <string>
#include <vector>

namespace Api
{
    struct Announcement
    {
        std::string title;
        std::string content;
        std::string date;
    };

    std::vector<Announcement> GetAnnouncements();
}
