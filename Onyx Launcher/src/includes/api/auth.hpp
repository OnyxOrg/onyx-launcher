#pragma once

#include <string>

namespace Api
{
	struct AuthResult
	{
		bool success = false;
		std::string token;
		std::string username;
		std::string role;
		std::string error;
	};

	// Performs POST /api/launcher/login
	AuthResult Login(const std::string& username, const std::string& password);

	// Performs POST /api/webapp/register
	AuthResult Register(const std::string& username, const std::string& password, const std::string& licenseKey);
}
