#pragma once

#include <string>

namespace ApiConfig
{
	// Primary API host (Discord bot API)
	inline const char* DefaultPrimary = "http://localhost:3000";

	inline std::string GetPrimaryBaseUrl()
	{
		return std::string(DefaultPrimary);
	}
}