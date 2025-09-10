#pragma once

#include <string>

namespace Credentials
{
	// Load saved username and decrypted password. Returns true if loaded.
	bool Load(std::string& outUsername, std::string& outPassword);

	// Save credentials (encrypts password). Returns true on success.
	bool Save(const std::string& username, const std::string& password);

	// Remove any saved credentials. Returns true on success or if none.
	bool Clear();
}


