#include "includes/core/views/login_view.hpp"
#include <thread>

void Views::RenderLogin(AppState& state, Alpha& alpha, Alpha& subalpha)
{
	const auto& window = GetCurrentWindow();
	if (state.isLoading || state.showPostLoginSpinner) SetOverlayActive(true);

	if (strlen(state.loginErrMsg) > 0)
	{
		SetCursorPos({ (window->Size.x - 220) / 2 + 5, 110 });
		PushFont(fonts->InterM[0]);
		draw->Text(state.loginErrMsg, colors::Red);
		PopFont();
	}

	// Smooth auto-fill + auto-signin on first frame if credentials exist
	static bool triedAutoSignIn = false;
	if (!triedAutoSignIn)
	{
		triedAutoSignIn = true;
		std::string su, sp;
		if (Credentials::Load(su, sp))
		{
			strncpy_s(state.loginUsr, sizeof(state.loginUsr), su.c_str(), _TRUNCATE);
			strncpy_s(state.loginPas, sizeof(state.loginPas), sp.c_str(), _TRUNCATE);
			state.remember = true;
			AuthFlow::KickoffAsyncLogin(state, state.loginUsr, state.loginPas);
		}
	}

	SetCursorPos({ (window->Size.x - 220) / 2, 135 });
	items->Input("Username", USER, "", state.loginUsr, _size(state.loginUsr), 0);

	SetCursorPosX((window->Size.x - 220) / 2);
	items->Input("Password", EYE_SLASHED, EYE, state.loginPas, _size(state.loginPas), ImGuiInputTextFlags_Password);

	SetCursorPosX((window->Size.x - 215) / 2);
	items->Checkbox("Remember me", &state.remember);

	if (state.isLoading && state.loginState == 2)
	{
		state.showPostLoginSpinner = true;
		state.postSpinnerEndTime = ImGui::GetTime() + AppState::kPostLoginSpinnerSeconds;
		state.isLoading = false;
		state.loginState = 0;
	}
	else if (state.isLoading && state.loginState == 3)
	{
		state.isLoading = false;
		state.loginState = 0;
		strncpy_s(state.loginErrMsg, sizeof(state.loginErrMsg), state.loginError.c_str(), _TRUNCATE);
		state.loginErrMsg[sizeof(state.loginErrMsg) - 1] = '\0';
	}

	std::string buttonLabel = "SIGN IN";
	SetCursorPosX((window->Size.x - 220) / 2);
	bool clickedSubmit = items->Button(buttonLabel, { window->Size.x - GetCursorPosX() * 2, 40 });
	bool enterPressed = IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter);
	if (clickedSubmit || enterPressed)
	{
		bool userEmpty = strlen(state.loginUsr) == 0;
		bool passEmpty = strlen(state.loginPas) == 0;
		items->SetInputError("Username", userEmpty);
		items->SetInputError("Password", passEmpty);
		if (!userEmpty && !passEmpty)
		{
			AuthFlow::KickoffAsyncLogin(state, state.loginUsr, state.loginPas);
		}
	}

	// Below the button: redirect to Sign up
	PushFont(fonts->InterM[0]);
	{
		std::string textLabel = "Don't you have an account? ";
		std::string redirectLabel = "Sign up";
		SetCursorPosX((window->Size.x - h->CT(textLabel + redirectLabel).x) / 2);
		draw->Text(textLabel, colors::Lwhite2);
		SameLine();
		if (items->TextButton(redirectLabel, redirectLabel, colors::Main))
		{
			ZeroMemory(state.loginUsr, sizeof(state.loginUsr));
			ZeroMemory(state.loginPas, sizeof(state.loginPas));
			ZeroMemory(state.registerUsr, sizeof(state.registerUsr));
			ZeroMemory(state.registerPas, sizeof(state.registerPas));
			ZeroMemory(state.licbuf, sizeof(state.licbuf));
			// Clear validation states so errors don't leak into the other form
			items->ClearInputError("Username");
			items->ClearInputError("Password");
			alpha.index = registerr;
		}
	}
	PopFont();

	PushFont(fonts->InterM[1]);
	SetCursorPosX((window->Size.x - h->CT("or").x) / 2);
	draw->Text("or", colors::Lwhite2);

	float length = 65.0f; 
	float distance = 15.0f; 
	float centerY = (GetCursorPosY() - GetStyle().ItemSpacing.y * 2) + h->CT("or").y * 0.5f; 

	window->DrawList->AddLine(window->Pos + vec2(window->Size.x * 0.5f - h->CT("or").x * 0.5f - length - distance, centerY), window->Pos + vec2(window->Size.x * 0.5f - h->CT("or").x * 0.5f - distance, centerY), h->CO(colors::Gray)); 
	window->DrawList->AddLine(window->Pos + vec2(window->Size.x * 0.5f + h->CT("or").x * 0.5f + distance, centerY), window->Pos + vec2(window->Size.x * 0.5f + h->CT("or").x * 0.5f + length + distance, centerY), h->CO(colors::Gray));

	// Legacy positioning as before (left/right under separator)
	float baseY = GetCursorPosY() - 5.0f;
	SetCursorPos({ (window->Size.x * 0.5f - h->CT("or").x * 0.5f - length - distance + window->Size.x * 0.5f - h->CT("or").x * 0.5f - distance) * 0.5f, baseY });
	(void)items->ImageButton("google", images->googleIcon, { 20, 20 });

	SameLine();
	SetCursorPosX((window->Size.x / 2 + h->CT("or").x / 2 + distance + length / 2) - 44);
	if (items->ImageButton("discord", images->discordIcon, { 21, 15 }))
	{
		std::thread([&] {
			ShellExecuteW(nullptr, L"open", L"https://discord.gg/7bcTMxvmAU", nullptr, nullptr, SW_SHOWNORMAL);
		}).detach();
	}
	PopFont();

	if (state.showPostLoginSpinner)
	{
		RenderLoadingOverlay();
		if (ImGui::GetTime() >= state.postSpinnerEndTime)
		{
			state.showPostLoginSpinner = false;
			alpha.index = home;
			subalpha.index = dashboard;
		}
	}
}

