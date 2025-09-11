#include "includes/core/app.hpp"
#include "includes/core/auth_flow.hpp"
#include "includes/core/views/login_view.hpp"
#include "includes/core/views/register_view.hpp"
#include "includes/core/overlay.hpp"
#include "includes/api/common.hpp"
#include <thread>
#include <thread>

static std::string FormatRole(const std::string&)
{
	return std::string("User");
}

vec4 GetRoleColor(const std::string&) { return colors::RoleMember; }

namespace App
{
	void RenderFrame(AppState& state)
	{
		const auto& window = GetCurrentWindow();
		SetOverlayActive(false);

		const static auto& alpha = std::make_unique<Alpha>(); alpha->Render();
		const static auto& subalpha = std::make_unique<Alpha>(); subalpha->Render();

		PushFont(fonts->Icons[0]);

		SetCursorPos({ window->Size.x - h->CT(CLOSE).x - 10, 8 });
		if (items->TextButton(CLOSE, "close_wnd"))
			exit(0);

		SameLine();

		SetCursorPos({ window->Size.x - h->CT(MINUS).x - 10 * 3, 7 });
		if (items->TextButton(MINUS, "minmize_wnd"))
			ShowWindow(::window->hWindow, SW_MINIMIZE);

		PopFont();

		PushStyleVar(ImGuiStyleVar_Alpha, alpha->alpha);

		if (alpha->tab < home)
		{
			window->DrawList->AddLine(window->Pos + vec2(0, 30), window->Pos + vec2(window->Size.x, 30), h->CO({ colors::Gray.x , colors::Gray.y, colors::Gray.z, 0.5 }));

			PushFont(fonts->Xirod);

			SetCursorPos({ (window->Size.x - h->CT("onyx").x) / 2, 56 });
			draw->Text("onyx", colors::White);

			PopFont();

			PushStyleVar(ImGuiStyleVar_ItemSpacing, { 0, 16 });
			{
				PushFont(fonts->InterM[1]);
				if (alpha->tab == login) Views::RenderLogin(state, *alpha, *subalpha);
				else Views::RenderRegister(state, *alpha);
				PopFont();

				// Removed inline redirect in App – moved to the specific login/register views for precise placement
			}
			PopStyleVar(); // itemspacing
		}

		if (alpha->tab == home)
		{
			gui->SizeOnChange = { 900, 600 };

			window->DrawList->AddLine(window->Pos + vec2(0, 70), window->Pos + vec2(160, 70), h->CO(colors::Gray));
			window->DrawList->AddLine(window->Pos + vec2(0, window->Size.y - 71), window->Pos + vec2(160, window->Size.y - 71), h->CO(colors::Gray));
			window->DrawList->AddLine(window->Pos + vec2(160, 0), window->Pos + vec2(160, window->Size.y), h->CO(colors::Gray));
			window->DrawList->AddImage((ImTextureID)images->logo, window->Pos + vec2(7, 7), window->Pos + vec2(67, 67));

			PushFont(fonts->InterM[4]);

			SetCursorPos({ 64, 25 });
			draw->Text("Onyx", colors::Main);

			PopFont();

			PushStyleVarY(ImGuiStyleVar_ItemSpacing, 10);
			PushFont(fonts->RennerM);

			SetCursorPos({ 15, 100 });
			if (items->Tab("Dashboard", HOME, subalpha->index == dashboard)) subalpha->index = dashboard;

			SetCursorPosX(15);
			if (items->Tab("Library", BOOKMARK, subalpha->index == library)) subalpha->index = library;

			PushFont(fonts->RennerM);
			SetCursorPos({ 15, window->Size.y - 58 });
			{
				const char* dispUser = state.authenticated
					? (state.discordUsername.empty() ? state.username.c_str() : state.discordUsername.c_str())
					: "relique";
				std::string roleDisplay = state.authenticated ? FormatRole(state.role) : std::string("User");
				if (items->Profile(dispUser, roleDisplay.c_str(), images->profilePic)) subalpha->index = profile;
			}
			PopFont();

			SetCursorPos({ 22, window->Size.y - 106 });
			Image((ImTextureID)images->discordIcon, { 20, 15 });

			SetCursorPos({ 22, window->Size.y - 106 });
			PushFont(fonts->discordSupportFont);
			if (items->TextButton("       Support", "dc_sp", colors::Lwhite))
			{
				std::thread([&] {
					ShellExecuteW(nullptr, L"open", L"https://discord.gg/7bcTMxvmAU", nullptr, nullptr, SW_SHOWNORMAL);
				}).detach();
			}

			PopFont();
			PopStyleVar(); // itemspacing

			PushStyleVar(ImGuiStyleVar_Alpha, subalpha->alpha);

			if (subalpha->tab == dashboard)
			{
				// Precompute welcome string so goto skip won't bypass initialization
				std::string welcomeMsg = std::string("Welcome back, ") + (state.authenticated ? state.username : std::string("relique"));
				if (state.showPostLoginSpinner)
				{
					RenderLoadingOverlay();
					if (ImGui::GetTime() >= state.postSpinnerEndTime)
					{
						state.showPostLoginSpinner = false;
					}
					else
					{
						goto SKIP_DASHBOARD_CONTENTS;
					}
				}
				SetCursorPos({ 190, 33 });
				custom->Banner("Dashboard", welcomeMsg.c_str(), images->banner);

				SetCursorPos({ 190, 230 });
				custom->BeginChild("News", { window->Size.x + 160 - GetCursorPosX() * 2, (window->Size.y - 200 - 50) - 10});
				{
					PushStyleVarY(ImGuiStyleVar_ItemSpacing, 12);

					items->Announcement("ONYX UPDATE 1.0", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", feature);
					items->Announcement("ONYX UPDATE 0.4", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", updated);
					items->Announcement("ONYX UPDATE 0.1", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", bugfix);
					items->Announcement("ONYX UPDATE 0.3", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", bugfix);
					
					Spacing();
					PopStyleVar(); // itemspacing
				}
				custom->EndChild();
SKIP_DASHBOARD_CONTENTS:;
			}

			if (subalpha->tab == library)
			{
				PushFont(fonts->InterS[1]);

				SetCursorPos({ 190, 25 });
				draw->Text("LIBRARY", colors::White);

				PopFont();

				PushStyleVar(ImGuiStyleVar_ItemSpacing, { 20, 20 });

				int column = 0;
				float startX = 190.0f;
				SetCursorPos({ startX, 70 });
				if (!state.ownedProducts.empty())
				{
					for (size_t i = 0; i < state.ownedProducts.size(); ++i)
					{
						const auto& p = state.ownedProducts[i];
						ProductStatus status = ProductStatus::Online;
						if (p.status == "offline") status = ProductStatus::Offline;
						else if (p.status == "updating") status = ProductStatus::Updating;

						vec4 timeLeftColor = colors::Main;
						std::string timeLeftText = FormatTimeLeftFromIso(p.expiresAt, timeLeftColor);

						(void)items->Product(p.name, timeLeftText, status, images->product, timeLeftColor);

						++column;
						if (column % 3 != 0)
						{
							SameLine();
						}
						else
						{
							SetCursorPosX(startX);
						}
					}
				}
				else
				{
					PushFont(fonts->InterM[2]);
					SetCursorPos({ startX, 80 });
					draw->Text("No products in your library yet.", colors::Lwhite2);
					PopFont();
				}

				PopStyleVar(); // itemspacing

			}

			if (subalpha->tab == profile)
			{
				PushFont(fonts->InterS[1]);

				SetCursorPos({ 190, 25 });
				draw->Text("PROFILE", colors::White);

				PopFont();

				PushStyleVar(ImGuiStyleVar_ItemSpacing, { 20, 20 });

				SetCursorPos({ 190, 70 });
				custom->BeginChild2("profile_showcase", { 300, 100 });
				{
					const auto& child = GetCurrentWindow();

					SetCursorPos({ 23, (child->Size.y - 60) / 2 });
					{
						vec2 avatarSize = { 60, 60 };
						vec2 avatarPos = child->DC.CursorPos;
						float rounding = avatarSize.y * 0.5f;
						child->DrawList->AddImageRounded((ImTextureID)images->profilePic, avatarPos, avatarPos + avatarSize, {}, { 1, 1 }, h->CO(colors::White), rounding);
						child->DrawList->AddRect(avatarPos, avatarPos + avatarSize, h->CO(colors::Gray), rounding, 0, 1.0f);
					}

					PushFont(fonts->InterM[2]);

					SetCursorPos({ 95, 30 });
					draw->Text(state.authenticated ? (state.discordUsername.empty() ? state.username.c_str() : state.discordUsername.c_str()) : "relique", colors::Main);

					SetCursorPos({ 95, 50 });
					PushFont(fonts->userUidFont);
					draw->Text(state.discordId.empty() ? "UID: Unknown" : (std::string("UID: ") + state.discordId).c_str(), colors::Lwhite2);
					PopFont();

					{
						std::string role = state.authenticated ? FormatRole(state.role) : std::string("User");
						PushFont(fonts->profileRoleFont);
						vec2 roleSize = h->CT(role);

						float rightPadding = 14;
						vec2 rectMin = { child->Pos.x + child->Size.x - (15 + roleSize.x) - rightPadding, child->Pos.y + (child->Size.y - 25) / 2 - 2 };
						vec2 rectMax = { child->Pos.x + child->Size.x - rightPadding, rectMin.y + 25 };
						child->DrawList->AddRectFilled(rectMin, rectMax, h->CO(colors::Gray2), 8);

						vec2 textPos = { rectMin.x + (rectMax.x - rectMin.x - roleSize.x) / 2, rectMin.y + (rectMax.y - rectMin.y - roleSize.y) / 2 };
						child->DrawList->AddText(textPos, h->CO(colors::RoleMember), role.c_str());
						PopFont();
					}

					PopFont();
				}
				custom->EndChild();

				SetCursorPosX(190);
				custom->BeginChild2("connect_discord", { 300, 165 });
				{
					PushFont(fonts->InterS[0]);

					SetCursorPos({ 15, 15 });
					draw->Text("Connect to Discord", colors::White);

					PopFont();

					PushFont(fonts->InterM[2]);

					SetCursorPos({ 15, 40 });
					draw->Text("Log in with your Discord Account to \nunlock your full profile: avatar, UID,\nregistration date, and more.", colors::Lwhite2);

					SetCursorPos({ 15, 115 });
					if (items->ButtonIcon("Link Discord Account", DISCORD, { 195, 35 }))
					{
						std::string authUrl = ApiConfig::BuildDiscordAuthorizeUrl(state.username);
						std::wstring wurl(authUrl.begin(), authUrl.end());
						std::thread([wurl] {
							ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
						}).detach();

						// Poll the bot for updated Discord link and refresh state
						std::thread([&state]() {
							for (int i = 0; i < 20; ++i)
							{
								std::this_thread::sleep_for(std::chrono::milliseconds(500));
								Api::UserInfo ui = Api::GetUserInfo(state.username);
								if (ui.ok && ui.discordConnected && !ui.discordId.empty())
								{
									state.discordId = ui.discordId;
									state.discordUsername = ui.discordUsername;
									state.discordAvatarHash = ui.discordAvatar;
									break;
								}
							}
						}).detach();
					}

					PopFont();
				}
				custom->EndChild();

				PopStyleVar(); // itemspacing
			}

			PopStyleVar(); // alpha
		}

		PopStyleVar(); // main alpha
	}
}

