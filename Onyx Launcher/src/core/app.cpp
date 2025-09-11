#include "includes/core/app.hpp"
#include "includes/core/auth_flow.hpp"
#include "includes/core/views/login_view.hpp"
#include "includes/core/views/register_view.hpp"
#include "includes/core/overlay.hpp"
#include "includes/api/common.hpp"
#include <thread>
#include <thread>

// Local fade-out state for Discord link success
static std::atomic<bool> g_showLinkSuccessFade{ false };
static double g_linkSuccessEndTime = 0.0;
static constexpr float kLinkSuccessFadeSeconds = 1.5f;
static std::atomic<bool> g_showCancelFade{ false };
static double g_cancelFadeEndTime = 0.0;
static constexpr float kLinkCancelFadeSeconds = 1.5f;

// Forward declare extended overlay API (header should also declare this)
void RenderLoadingOverlayEx(const char* label, float alphaMultiplier);

// Heuristic: detect if a browser window with the Discord OAuth title is open
static bool IsDiscordAuthWindowOpen()
{
	struct Ctx { bool found = false; } ctx;
	auto toLower = [](std::wstring s) {
		std::transform(s.begin(), s.end(), s.begin(), [](wchar_t c){ return (wchar_t)std::tolower(c); });
		return s;
	};

	EnumWindows([](HWND hwnd, LPARAM lp) -> BOOL {
		Ctx* ctx = reinterpret_cast<Ctx*>(lp);
		if (!IsWindowVisible(hwnd)) return TRUE;
		int len = GetWindowTextLengthW(hwnd);
		if (len <= 0) return TRUE;
		std::wstring title; title.resize(static_cast<size_t>(len) + 1);
		int got = GetWindowTextW(hwnd, &title[0], len + 1);
		if (got <= 0) return TRUE;
		title.resize(wcslen(title.c_str()));
		std::wstring t = title;
		std::transform(t.begin(), t.end(), t.begin(), [](wchar_t c){ return (wchar_t)std::tolower(c); });
		bool looksLikeDiscordOAuth = (t.find(L"discord | authorize access") != std::wstring::npos)
			|| (t.find(L"authorize access") != std::wstring::npos && t.find(L"discord") != std::wstring::npos)
			|| (t.find(L"oauth2") != std::wstring::npos && t.find(L"discord") != std::wstring::npos);
		if (looksLikeDiscordOAuth) { ctx->found = true; return FALSE; }
		return TRUE;
	}, reinterpret_cast<LPARAM>(&ctx));

	return ctx.found;
}

static std::string FormatRole(const std::string& role)
{
	if (role == "owner") return "Owner";
	if (role == "developer") return "Developer";
	if (role == "manager") return "Manager";
	if (role == "staff") return "Staff";
	return "User";
}

vec4 GetRoleColor(const std::string& role) {
	// Use profile header colors (web app) for the large profile badge
	if (role == "owner") return colors::RoleOwnerProfile;
	if (role == "developer") return colors::RoleDevProfile;
	if (role == "manager") return colors::RoleManagerProfile;
	if (role == "staff") return colors::RoleStaffProfile;
	return colors::RoleUserProfile;
}

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
				std::string displayName = state.authenticated ? (state.discordUsername.empty() ? state.username : state.discordUsername) : std::string("relique");
				std::string welcomeMsg = std::string("Welcome back, ") + displayName;
				if (state.showPostLoginSpinner)
				{
					RenderLoadingOverlay("Signing in...");
					if (ImGui::GetTime() >= state.postSpinnerEndTime)
					{
						state.showPostLoginSpinner = false;
					}
					else
					{
						goto SKIP_DASHBOARD_CONTENTS;
					}
				}
				// Note: Only show linking overlay in Profile tab per requirement
				SetCursorPos({ 190, 33 });
				custom->Banner("Dashboard", welcomeMsg.c_str(), images->banner);

				SetCursorPos({ 190, 230 });
				custom->BeginChild("News", { window->Size.x + 160 - GetCursorPosX() * 2, (window->Size.y - 200 - 50) - 10});
				{
					PushStyleVarY(ImGuiStyleVar_ItemSpacing, 12);

					items->Announcement("ONYX UPDATE 1.0", "Highlights: new dashboard, faster authentication, library auto-updates,\nperformance improvements, stability fixes, and a refreshed look.", "August 6, 2025", feature);
					items->Announcement("ONYX UPDATE 0.4", "Improvements: account linking, refined installer, reduced CPU usage,\nfixed crashes when switching tabs and clearer error messages.", "August 6, 2025", updated);
					items->Announcement("ONYX UPDATE 0.1", "Bug fixes: resolved login timeouts, license sync issues and UI clipping,\nmore reliable updater and a smoother first-run experience.", "August 6, 2025", bugfix);
					items->Announcement("ONYX UPDATE 0.3", "Maintenance: patched overlay memory leak, corrected theme colors,\nfixed broken dashboard links and minor layout glitches.", "August 6, 2025", bugfix);
					
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
				custom->BeginChild2("profile_showcase", { 330, 100 });
				{
					const auto& child = GetCurrentWindow();

					SetCursorPos({ 23, (child->Size.y - 60) / 2 });
					{
						vec2 avatarSize = { 60, 60 };
						vec2 avatarPos = child->DC.CursorPos;
						float rounding = avatarSize.y * 0.5f;
						ID3D11ShaderResourceView* tex = images->profilePic;
						child->DrawList->AddImageRounded((ImTextureID)tex, avatarPos, avatarPos + avatarSize, {}, { 1, 1 }, h->CO(colors::White), rounding);
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
						// If not linked to Discord, always show "User" (even if DB stores a higher role)
						std::string role = (!state.authenticated || state.discordId.empty()) ? std::string("User") : FormatRole(state.role);
						// In profile section, shorten Developer -> Dev
						if (role == "Developer") role = "Dev";
						PushFont(fonts->profileRoleFont);
						vec2 roleSize = h->CT(role);

						float rightPadding = 14;
						vec2 rectMin = { child->Pos.x + child->Size.x - (15 + roleSize.x) - rightPadding, child->Pos.y + (child->Size.y - 25) / 2 - 2 };
						vec2 rectMax = { child->Pos.x + child->Size.x - rightPadding, rectMin.y + 25 };
						child->DrawList->AddRectFilled(rectMin, rectMax, h->CO(colors::Gray2), 8);

						vec2 textPos = { rectMin.x + (rectMax.x - rectMin.x - roleSize.x) / 2, rectMin.y + (rectMax.y - rectMin.y - roleSize.y) / 2 };
						// Color based on role string we are displaying
						std::string raw = (!state.authenticated || state.discordId.empty()) ? std::string("user") : state.role;
						child->DrawList->AddText(textPos, h->CO(GetRoleColor(raw)), role.c_str());
						PopFont();
					}

					PopFont();
				}
				custom->EndChild();

				SetCursorPosX(190);
				custom->BeginChild2("connect_discord", { 330, 165 });
				{
					if (state.discordId.empty())
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
							state.isLinkingDiscord = true;
							std::string authUrl = ApiConfig::BuildDiscordAuthorizeUrl(state.username);
							std::wstring wurl(authUrl.begin(), authUrl.end());
							std::thread([wurl] {
								ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
							}).detach();

							// Start a watcher that closes overlay as soon as the auth window is closed
							std::thread([&state]() {
								bool sawWindow = false;
								for (;;) {
									if (g_showLinkSuccessFade) break; // already succeeded
									bool open = IsDiscordAuthWindowOpen();
									if (open) sawWindow = true;
									if (sawWindow && !open) { 
										g_showCancelFade = true; 
										g_cancelFadeEndTime = ImGui::GetTime() + kLinkCancelFadeSeconds; 
										state.isLinkingDiscord = false; 
										break; 
									}
									std::this_thread::sleep_for(std::chrono::milliseconds(120));
								}
							}).detach();

							// Poll for updated link state (faster interval) and stop immediately on success or watcher close
							std::thread([&state]() {
								for (int i = 0; i < 40; ++i)
								{
									std::this_thread::sleep_for(std::chrono::milliseconds(250));
									Api::UserInfo ui = Api::GetUserInfo(state.username);
									if (ui.ok && ui.discordConnected && !ui.discordId.empty())
									{
										state.discordId = ui.discordId;
										state.discordUsername = ui.discordUsername;
										// After link established, sync role immediately
										auto sync = Api::SyncRole(state.username, state.discordId);
										if (sync.ok && !sync.role.empty()) state.role = sync.role;
										// Trigger a short success fade-out (local globals)
										g_showLinkSuccessFade = true;
										g_linkSuccessEndTime = ImGui::GetTime() + kLinkSuccessFadeSeconds;
										state.isLinkingDiscord = false;
										g_showCancelFade = false; // cancel any pending cancel-fade
										break;
									}
									if (!state.isLinkingDiscord) break; // watcher closed it
								}
							}).detach();
						}
						PopFont();
					}
					else
					{
						PushFont(fonts->InterS[0]);
						SetCursorPos({ 15, 15 });
						draw->Text("Discord Linked", colors::White);
						PopFont();

						PushFont(fonts->InterM[1]);
						SetCursorPos({ 15, 40 });
						draw->Text("Your Discord account is currently connected", colors::Lwhite2);
						SetCursorPos({ 15, 58 });
						draw->Text("to Onyx. You can unlink at any time.", colors::Lwhite2);
						SetCursorPos({ 15, 110 });
						if (items->ButtonDangerIcon("Unlink Discord Account", DISCORD, { 230, 35 }))
						{
							std::thread([&state]() {
								Api::UnlinkDiscord(state.username);
								// Refresh local state
								Api::UserInfo ui = Api::GetUserInfo(state.username);
								state.discordId = ui.discordConnected ? ui.discordId : std::string();
								state.discordUsername = ui.discordConnected ? ui.discordUsername : std::string();
								// After unlink, role may drop to User; refresh via role sync
								auto sync = Api::SyncRole(state.username, state.discordId);
								if (sync.ok && !sync.role.empty()) state.role = sync.role;
							}).detach();
						}
						PopFont();
					}
				}
				custom->EndChild();

				PopStyleVar(); // itemspacing
			}


			// Only in Profile tab: show linking overlay and fades
			if (subalpha->tab == profile)
			{
				if (state.isLinkingDiscord)
				{
					RenderLoadingOverlay("Linking Discord...");
				}
				else if (g_showLinkSuccessFade)
				{
					float remain = (float)(g_linkSuccessEndTime - ImGui::GetTime());
					float a = remain <= 0.0f ? 0.0f : (remain / kLinkSuccessFadeSeconds);
					if (a > 0.0f)
						RenderLoadingOverlayEx("Linked!", a);
					else
						g_showLinkSuccessFade = false;
				}
				else if (g_showCancelFade)
				{
					float remain = (float)(g_cancelFadeEndTime - ImGui::GetTime());
					float a = remain <= 0.0f ? 0.0f : (remain / kLinkCancelFadeSeconds);
					if (a > 0.0f)
						RenderLoadingOverlayEx("Cancelled", a);
					else
						g_showCancelFade = false;
				}
			}

			PopStyleVar(); // alpha
		}

		PopStyleVar(); // main alpha
	}
}