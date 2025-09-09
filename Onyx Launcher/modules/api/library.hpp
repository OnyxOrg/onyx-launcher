#pragma once

#include <string>
#include <vector>
#include <map>

namespace Api
{
	struct LibraryProduct
	{
		std::string name;
		std::string status;   // "online" | "offline" | "updating"
		std::string expiresAt; // ISO string or empty if lifetime
		std::string durationLabel; // Human label (e.g., "Lifetime", "3 days left")
	};

	// GET {webapp}/api/library/:username
	// Returns owned products for the given account from the webapp server
	std::vector<LibraryProduct> GetUserLibrary(const std::string& username);
}


