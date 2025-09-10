#include "includes/core/views/register_view.hpp"

void Views::RenderRegister(AppState& state, Alpha& alpha)
{
	const auto& window = GetCurrentWindow();
	SetCursorPos({ (window->Size.x - 220) / 2, 135 });
	items->Input("Username", USER, "", state.registerUsr, _size(state.registerUsr), 0);
	SetCursorPosX((window->Size.x - 220) / 2);
	items->Input("Password", EYE_SLASHED, EYE, state.registerPas, _size(state.registerPas), ImGuiInputTextFlags_Password);

	SetCursorPosX((window->Size.x - 220) / 2);
	items->Input("License key", KEY, "", state.licbuf, _size(state.licbuf));

	std::string buttonLabel = "SIGN UP";
	SetCursorPosX((window->Size.x - 220) / 2);
	bool clickedSubmit = items->Button(buttonLabel, { window->Size.x - GetCursorPosX() * 2, 40 });
	bool enterPressed = IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter);
	if (clickedSubmit || enterPressed)
	{
		bool userEmpty = strlen(state.registerUsr) == 0;
		bool passEmpty = strlen(state.registerPas) == 0;
		bool licEmpty = strlen(state.licbuf) == 0;
		items->SetInputError("Username", userEmpty);
		items->SetInputError("Password", passEmpty);
		items->SetInputError("License key", licEmpty);
		if (!userEmpty && !passEmpty && !licEmpty)
		{
			// Call API to register
			std::string username(state.registerUsr);
			std::string password(state.registerPas);
			std::string key(state.licbuf);
			Api::AuthResult rr = Api::Register(username, password, key);
			if (!rr.success)
			{
				strncpy_s(state.registerErrMsg, sizeof(state.registerErrMsg), rr.error.c_str(), _TRUNCATE);
				state.registerErrMsg[sizeof(state.registerErrMsg) - 1] = '\0';
			}
			else
			{
				// On success, go back to login and prefill username
				ZeroMemory(state.loginUsr, sizeof(state.loginUsr));
				ZeroMemory(state.loginPas, sizeof(state.loginPas));
				strncpy_s(state.loginUsr, sizeof(state.loginUsr), username.c_str(), _TRUNCATE);
				ZeroMemory(state.registerErrMsg, sizeof(state.registerErrMsg));
				alpha.index = login;
			}
		}
	}

	// Below button: redirect to Sign in
	PushFont(fonts->InterM[0]);
	{
		std::string textLabel = "Already have an account? ";
		std::string redirectLabel = "Sign in";
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
			items->ClearInputError("Username");
			items->ClearInputError("Password");
			items->ClearInputError("License key");
			ZeroMemory(state.registerErrMsg, sizeof(state.registerErrMsg));
			alpha.index = login;
		}
	}
	PopFont();

	// Error area below the redirect text
	if (strlen(state.registerErrMsg) > 0)
	{
		std::string msg(state.registerErrMsg);
		float maxWidth = window->Size.x - 40.0f;
		std::string line1 = msg;
		std::string line2;

		if (h->CT(msg).x > maxWidth)
		{
			std::string current;
			size_t lastBreak = std::string::npos;
			float width = 0.0f;
			for (size_t i = 0; i < msg.size(); ++i)
			{
				char c = msg[i];
				current.push_back(c);
				if (c == ' ') lastBreak = i;
				width = h->CT(current).x;
				if (width > maxWidth)
				{
					size_t cut = (lastBreak != std::string::npos) ? lastBreak : i;
					line1 = msg.substr(0, cut);
					line2 = msg.substr(cut + 1);
					break;
				}
			}
		}

		PushFont(fonts->InterM[0]);
		float w1 = h->CT(line1).x;
		SetCursorPosX((window->Size.x - w1) / 2);
		draw->Text(line1, colors::Red);
		if (!line2.empty())
		{
			const float tightenPx = -10.0f; // negative to tighten line spacing
			SetCursorPosY(GetCursorPosY() + tightenPx);
			float w2 = h->CT(line2).x;
			SetCursorPosX((window->Size.x - w2) / 2);
			draw->Text(line2, colors::Red);
		}
		PopFont();
	}
}