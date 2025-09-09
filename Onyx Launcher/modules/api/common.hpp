#pragma once

#include <string>

namespace ApiConfig
{
	// Primary API host (Discord bot API)
	inline const char* DefaultPrimary = "http://localhost:3000";

	// Webapp API host (express app serving account/library)
	inline const char* DefaultWebApp = "http://localhost:3100";

	inline std::string GetPrimaryBaseUrl()
	{
		return std::string(DefaultPrimary);
	}

	inline std::string GetWebAppBaseUrl()
	{
		return std::string(DefaultWebApp);
	}

	inline std::string GetFallbackBaseUrl()
	{
		return std::string();
	}
}


