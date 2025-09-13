#include "includes/core/views/unlink-modal.hpp"
#include "includes/core/items/overlay.hpp"
#include "includes/core/utils/image_loader.hpp"
#include <thread>

using namespace ImGui;

namespace UI
{
	void RenderUnlinkModal(AppState& state)
	{
		if (!state.showUnlinkPasswordModal)
			return;

		OpenPopup("unlink_modal");
		PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
		PushStyleVar(ImGuiStyleVar_WindowPadding, { 18, 16 });
		PushStyleColor(ImGuiCol_ModalWindowDimBg, ImVec4(0.0f, 0.0f, 0.0f, 0.70f));
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
				state.showUnlinkPasswordModal = false;
				CloseCurrentPopup();
			}
			SameLine();
			if (items->Button("Cancel", { 150, 34 }))
			{
				state.showUnlinkPasswordModal = false;
				CloseCurrentPopup();
			}
			EndPopup();
		}
		PopStyleColor();
		PopStyleVar(2);
	}
}


