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

	// Helper used internally by launcher to optionally enrich statuses from the webapp
	// Returns a map of lowercased product name -> lowercased status
	std::map<std::string, std::string> TryFetchStatusesFromWebApp(const std::string& username);

	// GET {webapp}/api/library/:username
	// Returns owned products for the given account from the webapp server
	std::vector<LibraryProduct> GetUserLibrary(const std::string& username);
}


