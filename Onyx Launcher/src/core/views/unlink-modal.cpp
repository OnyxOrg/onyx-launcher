#include "includes/core/views/unlink-modal.hpp"
#include "includes/core/items/overlay.hpp"
#include "includes/core/utils/image_loader.hpp"
#include <thread>

using namespace ImGui;

namespace UI
{
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
		PushStyleVar(ImGuiStyleVar_WindowPadding, { 18, 16 });
		PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 0.0f, 0.0f, 0.70f * a));
		SetNextWindowPos(GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (BeginPopupModal("unlink_modal", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove))
		{
			PushFont(fonts->InterM[2]);
			draw->Text("Set password to unlink", colors::Main);
			PopFont();
			Dummy({ 0, 8 });
			SetCursorPosX(GetCursorPosX());
			items->Input("New password", EYE_SLASHED, EYE, state.unlinkPassBuf, _size(state.unlinkPassBuf), ImGuiInputTextFlags_Password);
			if (state.unlinkErrMsg[0] != '\\0') { Text("%s", state.unlinkErrMsg); }
			Dummy({ 0, 12 });
			if (items->ButtonDangerIcon("Unlink", "", { 150, 34 }))
			{
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
				// Begin fade-out; we'll close when animation reaches 0
				s_fadingOut = true;
				s_animStart = GetTime();
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
		PopStyleColor();
		PopStyleVar(3); // includes Alpha + WindowRounding + WindowPadding

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


