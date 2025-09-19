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
	char registerErrMsg[128] = {0};

	// Auth/session
	std::string token;
	std::string username;
	std::string role;
	bool authenticated = false;
	std::vector<Api::LibraryProduct> ownedProducts;

	// Discord linked state
	std::string discordId;
	std::string discordUsername;
	std::string discordAvatarHash; // hash only
	ID3D11ShaderResourceView* avatarTexture = nullptr; // created from CDN when available
	std::string createdVia; // 'manual', 'discord', 'launcher', 'web app'

	// Async login states
	std::atomic<int> loginState{ 0 }; // 0 idle, 1 loading, 2 success, 3 failure
	bool isLoading = false;
	bool isLinkingDiscord = false;
	bool showUnlinkPasswordModal = false;
	char unlinkPassBuf[42] = {0};
	bool unlinkPassVisible = false;
	char unlinkErrMsg[128] = {0};
	bool showUnlinkConfirmationModal = false;
	std::string loginError;
	bool showPostLoginSpinner = false;
	double postSpinnerEndTime = 0.0;
	static constexpr float kPostLoginSpinnerSeconds = 2.5f;

	// UI preferences
	bool remember = false;

};


