#include "includes/core/time_util.hpp"

static bool ParseIsoParts(const std::string& iso, int& year, int& mon, int& day, int& hour, int& min, int& sec)
{
	int matched = ::sscanf_s(iso.c_str(), "%d-%d-%dT%d:%d:%d", &year, &mon, &day, &hour, &min, &sec);
	if (matched < 5) return false; // need at least Y-M-D T H:M
	if (matched == 5) sec = 0;
	return true;
}

bool ParseIsoUtcToTimeT(const std::string& iso, std::time_t& outUtc)
{
	int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;
	if (!ParseIsoParts(iso, year, mon, day, hour, min, sec)) return false;
	std::tm tm{};
	tm.tm_year = year - 1900;
	tm.tm_mon = mon - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;
	std::time_t t = _mkgmtime(&tm);
	if (t == (std::time_t)-1) return false;
	outUtc = t;
	return true;
}

std::string FormatTimeLeftFromIso(const std::string& iso, vec4& outColor)
{
	if (iso.empty())
	{
		outColor = colors::Green;
		return std::string("Lifetime");
	}

	std::time_t targetUtc;
	if (!ParseIsoUtcToTimeT(iso, targetUtc))
	{
		outColor = colors::Main;
		return std::string("Active");
	}

	std::time_t nowUtc = std::time(nullptr);
	long long diffSec = static_cast<long long>(targetUtc) - static_cast<long long>(nowUtc);
	if (diffSec <= 0)
	{
		outColor = colors::Red;
		return std::string("Expired");
	}

	long long minutes = diffSec / 60;
	long long hours = minutes / 60;
	long long days = hours / 24;
	long long months = days / 30;
	long long years = days / 365;

	std::string text;
	if (years >= 1)
		text = std::to_string(years) + " year" + (years != 1 ? "s" : "") + " left";
	else if (months >= 1)
		text = std::to_string(months) + " month" + (months != 1 ? "s" : "") + " left";
	else if (days >= 1)
		text = std::to_string(days) + " day" + (days != 1 ? "s" : "") + " left";
	else if (hours >= 1)
		text = std::to_string(hours) + " hour" + (hours != 1 ? "s" : "") + " left";
	else
		text = std::to_string((long long)std::max<long long>(1, minutes)) + " min" + (minutes != 1 ? "s" : "") + " left";

	// thresholds
	if (days > 7) outColor = colors::Green;
	else if (hours > 23 || (days >= 1 && days <= 7)) outColor = colors::Yellow;
	else outColor = colors::Red;

	return text;
}


