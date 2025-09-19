#include "includes/core/views/unlink-modal.hpp"
#include "includes/core/items/overlay.hpp"
#include "includes/core/utils/image_loader.hpp"
#include <thread>
#include <cstring>
#include <cctype>

using namespace ImGui;

namespace UI
{
	void RenderUnlinkConfirmationModal(AppState& state)
	{
		// Local animation state
		static bool s_modalOpen = false;          // true while rendering (including fade-out)
		static bool s_fadingOut = false;          // whether we are fading out
		static double s_animStart = 0.0;          // animation start time
		static const float kAnimDuration = 0.18f; // seconds

		// Trigger open: when external state requests it and we are not already open
		if (state.showUnlinkConfirmationModal && !s_modalOpen)
		{
			s_modalOpen = true;
			s_fadingOut = false;
			s_animStart = GetTime();
			OpenPopup("unlink_confirmation_modal");
		}

		if (!s_modalOpen)
			return;

		// Compute animation alpha based on current phase
		float t = (float)(GetTime() - s_animStart);
		float p = ImClamp(t / kAnimDuration, 0.0f, 1.0f);
		float a = s_fadingOut ? (1.0f - p) : p; // fade-in to 1, fade-out to 0

		PushStyleVar(ImGuiStyleVar_Alpha, a);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		PushStyleVar(ImGuiStyleVar_WindowPadding, { 18, 6 });
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 0.0f, 0.0f, 0.70f * a));
		PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.08f)); // subtle border
		SetNextWindowPos(GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		// Fix modal size
		const float kButtonWidth = 120.0f;
		const float kButtonsSpacing = GetStyle().ItemSpacing.x;
		const float kWindowPad = 18.0f;
		float modalWidth = (kButtonWidth * 2.0f) + kButtonsSpacing + (kWindowPad * 2.0f);
		const float modalHeight = 140.0f;
		SetNextWindowSize(ImVec2(modalWidth, modalHeight), ImGuiCond_Always);
		if (BeginPopupModal("unlink_confirmation_modal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			// Allow closing by clicking outside
			bool requestClose = false;
			if (IsMouseClicked(ImGuiMouseButton_Left) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
				requestClose = true;
			if (requestClose && !s_fadingOut)
			{
				s_fadingOut = true;
				s_animStart = GetTime();
			}

			Dummy({ 0, 8 }); // shift header down slightly
			PushFont(fonts->InterM[2]);
			draw->Text("Unlink Discord Account?", colors::Main);
			PopFont();
			Dummy({ 0, 4 }); // Reduced from 8 to 4 to move text up
			
			// Warning message
			PushFont(fonts->InterM[1]);
			draw->Text("Are you sure you want to unlink your", colors::Lwhite2);
			SetCursorPosY(GetCursorPosY() - 4.0f); // Increased from -2.0f to -4.0f to move second line up more
			draw->Text("Discord account from Onyx?", colors::Lwhite2);
			PopFont();
			
			// Keep buttons at a fixed Y
			const float kButtonHeight = 34.0f;
			float bottomPadding = GetStyle().WindowPadding.y;
			const float kLiftUp = 8.0f;
			float buttonsTop = GetWindowHeight() - bottomPadding - kButtonHeight - kLiftUp;
			SetCursorPosY(buttonsTop);

			// Enter key triggers Unlink action
			if (IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter))
			{
				state.showUnlinkConfirmationModal = false;
				// Direct unlink for non-Discord created accounts
				if (state.avatarTexture) { state.avatarTexture->Release(); state.avatarTexture = nullptr; }
				state.discordAvatarHash.clear();
				std::thread([&state]() {
					Api::UnlinkDiscord(state.username);
					Api::UserInfo ui = Api::GetUserInfo(state.username);
					state.discordId = ui.discordConnected ? ui.discordId : std::string();
					state.discordUsername = ui.discordConnected ? ui.discordUsername : std::string();
					state.discordAvatarHash.clear();
					if (state.avatarTexture) { state.avatarTexture->Release(); state.avatarTexture = nullptr; }
					auto sync = Api::SyncRole(state.username, state.discordId);
					if (sync.ok && !sync.role.empty()) state.role = sync.role;
				}).detach();
				s_fadingOut = true;
				s_animStart = GetTime();
			}

			if (items->ButtonDangerIcon("Unlink", "", { kButtonWidth, kButtonHeight }))
			{
				state.showUnlinkConfirmationModal = false;
				// Direct unlink for non-Discord created accounts
				if (state.avatarTexture) { state.avatarTexture->Release(); state.avatarTexture = nullptr; }
				state.discordAvatarHash.clear();
				std::thread([&state]() {
					Api::UnlinkDiscord(state.username);
					Api::UserInfo ui = Api::GetUserInfo(state.username);
					state.discordId = ui.discordConnected ? ui.discordId : std::string();
					state.discordUsername = ui.discordConnected ? ui.discordUsername : std::string();
					state.discordAvatarHash.clear();
					if (state.avatarTexture) { state.avatarTexture->Release(); state.avatarTexture = nullptr; }
					auto sync = Api::SyncRole(state.username, state.discordId);
					if (sync.ok && !sync.role.empty()) state.role = sync.role;
				}).detach();
				s_fadingOut = true;
				s_animStart = GetTime();
			}
			SameLine();
			if (items->Button("Cancel", { kButtonWidth, kButtonHeight }))
			{
				// Start fade-out on cancel
				s_fadingOut = true;
				s_animStart = GetTime();
			}
			EndPopup();
		}
		PopStyleColor(2);
		PopStyleVar(4); // includes Alpha + WindowRounding + WindowPadding + BorderSize

		// When fade-out completes, close the popup and clear external flag
		if (s_fadingOut)
		{
			float t2 = (float)(GetTime() - s_animStart);
			if (t2 >= kAnimDuration)
			{
				// finalize close
				s_modalOpen = false;
				s_fadingOut = false;
				state.showUnlinkConfirmationModal = false;
				CloseCurrentPopup();
			}
		}
	}

	void RenderUnlinkModal(AppState& state)
	{
		// Local animation state
		static bool s_modalOpen = false;          // true while rendering (including fade-out)
		static bool s_fadingOut = false;          // whether we are fading out
		static double s_animStart = 0.0;          // animation start time
		static const float kAnimDuration = 0.18f; // seconds

		// Trigger open: when external state requests it and we are not already open
		if (state.showUnlinkPasswordModal && !s_modalOpen)
		{
			s_modalOpen = true;
			s_fadingOut = false;
			s_animStart = GetTime();
			// Reset previous state so reopening starts clean
			ZeroMemory(state.unlinkErrMsg, sizeof(state.unlinkErrMsg));
			items->ClearInputError("New password");
			ZeroMemory(state.unlinkPassBuf, sizeof(state.unlinkPassBuf));
			OpenPopup("unlink_modal");
		}

		if (!s_modalOpen)
			return;

		// Compute animation alpha based on current phase
		float t = (float)(GetTime() - s_animStart);
		float p = ImClamp(t / kAnimDuration, 0.0f, 1.0f);
		float a = s_fadingOut ? (1.0f - p) : p; // fade-in to 1, fade-out to 0

		PushStyleVar(ImGuiStyleVar_Alpha, a);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		PushStyleVar(ImGuiStyleVar_WindowPadding, { 18, 6 });
		PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
		PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 0.0f, 0.0f, 0.70f * a));
		PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 0.08f)); // subtle border
		SetNextWindowPos(GetMainViewport()->GetCenter(), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
		// Fix modal size to exactly fit the two buttons width + window padding
		const float kButtonWidth = 150.0f; // must match button calls below
		const float kButtonsSpacing = GetStyle().ItemSpacing.x;
		const float kWindowPad = 18.0f; // ImGuiStyleVar_WindowPadding set above
		float modalWidth = (kButtonWidth * 2.0f) + kButtonsSpacing + (kWindowPad * 2.0f);
		const float modalHeight = 175.0f; // tighter fixed height for a more symmetric layout
		SetNextWindowSize(ImVec2(modalWidth, modalHeight), ImGuiCond_Always);
		if (BeginPopupModal("unlink_modal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			// Allow closing by clicking outside; Enter will trigger Unlink instead
			bool requestClose = false;
			if (IsMouseClicked(ImGuiMouseButton_Left) && !IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
				requestClose = true;
			if (requestClose && !s_fadingOut)
			{
				s_fadingOut = true;
				s_animStart = GetTime();
			}

			Dummy({ 0, 6 }); // shift header down slightly
			PushFont(fonts->InterM[2]);
			draw->Text("Set password to unlink", colors::Main);
			PopFont();
			Dummy({ 0, 8 });
			SetCursorPosX(GetCursorPosX());
			items->Input("New password", EYE_SLASHED, EYE, state.unlinkPassBuf, _size(state.unlinkPassBuf), ImGuiInputTextFlags_Password);
			if (state.unlinkErrMsg[0] != '\\0')
			{
				// Word-wrap to two lines max within modal content width
				std::string msg(state.unlinkErrMsg);
				float maxWidth = GetCurrentWindow()->Size.x - 40.0f; // padding safety
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
				// Use smaller font for the error text only so buttons aren't affected
				PushFont(fonts->InterM[0]);
				draw->Text(line1, colors::Red);
				if (!line2.empty())
				{
					SetCursorPosY(GetCursorPosY() - 2.0f); // tighten spacing slightly
					draw->Text(line2, colors::Red);
				}
				PopFont();
			}
			// Keep buttons at a fixed Y with no extra space below
			const float kButtonHeight = 34.0f;
			// Compute bottom position then lift up by a custom offset
			float bottomPadding = GetStyle().WindowPadding.y; // currently 6
			const float kLiftUp = 8.0f; // move buttons up by N pixels
			float buttonsTop = GetWindowHeight() - bottomPadding - kButtonHeight - kLiftUp;
			SetCursorPosY(buttonsTop);

			auto hasNonWhitespace = [](const char* s) -> bool {
				if (!s) return false;
				for (const char* p = s; *p; ++p) { if (!std::isspace((unsigned char)*p)) return true; }
				return false;
			};

			auto triggerUnlink = [&]() {
				bool hasPassword = hasNonWhitespace(state.unlinkPassBuf);
				if (!hasPassword)
				{
					strcpy_s(state.unlinkErrMsg, sizeof(state.unlinkErrMsg), "Please enter a password");
					items->SetInputError("New password", true);
					return;
				}
				// Registration password policy: at least 7 chars and includes a number
				size_t len = strlen(state.unlinkPassBuf);
				bool hasDigit = false; for (size_t i = 0; i < len; ++i) { if (std::isdigit((unsigned char)state.unlinkPassBuf[i])) { hasDigit = true; break; } }
				if (len < 7 || !hasDigit)
				{
					strcpy_s(state.unlinkErrMsg, sizeof(state.unlinkErrMsg), "Password must be at least 7 characters and include a number");
					items->SetInputError("New password", true);
					return;
				}
				std::string pass(state.unlinkPassBuf);
				std::thread([&state, pass]() {
					Api::UnlinkDiscord(state.username, pass);
					Api::UserInfo ui = Api::GetUserInfo(state.username);
					state.discordId = ui.discordConnected ? ui.discordId : std::string();
					state.discordUsername = ui.discordConnected ? ui.discordUsername : std::string();
					state.discordAvatarHash.clear();
					if (state.avatarTexture) { state.avatarTexture->Release(); state.avatarTexture = nullptr; }
					auto sync = Api::SyncRole(state.username, state.discordId);
					if (sync.ok && !sync.role.empty()) state.role = sync.role;
				}).detach();
				s_fadingOut = true;
				s_animStart = GetTime();
			};

			// Enter key triggers Unlink action
			if (IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter))
				triggerUnlink();

			if (items->ButtonDangerIcon("Unlink", "", { 150, 34 }))
			{
				triggerUnlink();
			}
			SameLine();
			if (items->Button("Cancel", { 150, 34 }))
			{
				// Start fade-out on cancel
				s_fadingOut = true;
				s_animStart = GetTime();
			}
			EndPopup();
		}
		PopStyleColor(2);
		PopStyleVar(4); // includes Alpha + WindowRounding + WindowPadding + BorderSize

		// When fade-out completes, close the popup and clear external flag
		if (s_fadingOut)
		{
			float t2 = (float)(GetTime() - s_animStart);
			if (t2 >= kAnimDuration)
			{
				// finalize close
				s_modalOpen = false;
				s_fadingOut = false;
				state.showUnlinkPasswordModal = false;
				CloseCurrentPopup();
			}
		}
	}
}


