#pragma once

#include "../main.hpp"
#include <string>
#include "colors.hpp"

// Parse ISO-8601 to UTC time_t
bool ParseIsoUtcToTimeT(const std::string& iso, std::time_t& outUtc);

// Compute human-friendly time left and choose color
std::string FormatTimeLeftFromIso(const std::string& iso, vec4& outColor);


