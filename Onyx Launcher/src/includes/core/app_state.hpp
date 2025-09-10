#pragma once

#include "main.hpp"
#include "../api/auth.hpp"
#include "../api/library.hpp"

struct AppState
{
	// UI input buffers
	char loginUsr[42] = {0};
	char loginPas[42] = {0};
	char registerUsr[42] = {0};
	char registerPas[42] = {0};
	char licbuf[42] = {0};

	char loginErrMsg[128] = {0};

	// Auth/session
	std::string token;
	std::string username;
	std::string role;
	bool authenticated = false;
	std::vector<Api::LibraryProduct> ownedProducts;

	// Async login states
	std::atomic<int> loginState{ 0 }; // 0 idle, 1 loading, 2 success, 3 failure
	bool isLoading = false;
	std::string loginError;
	bool showPostLoginSpinner = false;
	double postSpinnerEndTime = 0.0;
	static constexpr float kPostLoginSpinnerSeconds = 2.5f;

	// UI preferences
	bool remember = false;
};


