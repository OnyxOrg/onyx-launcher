#include "includes\\core\\items\\custom.hpp"
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdio>
#include <atomic>
#include "includes\\api\\auth.hpp"
#include "includes\\api\\library.hpp"
#include <cstring>

char loginUsr[42]
    ,loginPas[42]
    ,registerUsr[42]
    ,registerPas[42]
    ,licbuf[42]; // + 1 byte for null character

static char loginErrMsg[128];

static std::string g_token;
static std::string g_username;
static std::string g_role;
static bool g_authenticated = false;
static std::vector<Api::LibraryProduct> g_ownedProducts;
static std::atomic<int> g_loginState{ 0 }; // 0 idle, 1 loading, 2 success, 3 failure
static bool g_isLoading = false;
static std::string g_loginError;
static bool g_showPostLoginSpinner = false;
static double g_postSpinnerEndTime = 0.0;
static constexpr float kPostLoginSpinnerSeconds = 2.5f; // spinner duration (seconds)

static std::string FormatRole(const std::string&)
{
    return std::string("User");
}

vec4 GetRoleColor(const std::string&) { return colors::RoleMember; }

// Parse an ISO-8601 timestamp like "YYYY-MM-DDTHH:MM:SS[.mmm]Z" to UTC time_t
static bool ParseIsoUtcToTimeT(const std::string& iso, std::time_t& outUtc)
{
	int year = 0, mon = 0, day = 0, hour = 0, min = 0, sec = 0;
	int matched = ::sscanf_s(iso.c_str(), "%d-%d-%dT%d:%d:%d", &year, &mon, &day, &hour, &min, &sec);
	if (matched < 5) return false; // need at least Y-M-D T H:M
	if (matched == 5) sec = 0;
	std::tm tm{};
	tm.tm_year = year - 1900;
	tm.tm_mon = mon - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min = min;
	tm.tm_sec = sec;
	std::time_t t = _mkgmtime(&tm);
	if (t == (std::time_t)-1) return false;
	outUtc = t;
	return true;
}

// Format remaining time similarly to webapp (no special rounding)
static std::string FormatTimeLeftFromIso(const std::string& iso, vec4& outColor)
{
	if (iso.empty())
	{
		outColor = colors::Green;
		return std::string("Lifetime");
	}

	std::time_t targetUtc;
	if (!ParseIsoUtcToTimeT(iso, targetUtc))
	{
		outColor = colors::Main;
		return std::string("Active");
	}

	std::time_t nowUtc = std::time(nullptr);
	long long diffSec = static_cast<long long>(targetUtc) - static_cast<long long>(nowUtc);
	if (diffSec <= 0)
	{
		outColor = colors::Red;
		return std::string("Expired");
	}

	long long minutes = diffSec / 60;
	long long hours = minutes / 60;
	long long days = hours / 24;
	long long months = days / 30;
	long long years = days / 365;

	std::string text;
	if (years >= 1)
		text = std::to_string(years) + " year" + (years != 1 ? "s" : "") + " left";
	else if (months >= 1)
		text = std::to_string(months) + " month" + (months != 1 ? "s" : "") + " left";
	else if (days >= 1)
		text = std::to_string(days) + " day" + (days != 1 ? "s" : "") + " left";
	else if (hours >= 1)
		text = std::to_string(hours) + " hour" + (hours != 1 ? "s" : "") + " left";
	else
		text = std::to_string((long long)std::max<long long>(1, minutes)) + " min" + (minutes != 1 ? "s" : "") + " left";

	// Color thresholds: >7 days = green, 1-7 days = yellow, <=23 hours = red
	if (days > 7) outColor = colors::Green;
	else if (hours > 23 || (days >= 1 && days <= 7)) outColor = colors::Yellow;
	else outColor = colors::Red;

	return text;
}

// Simple fullscreen loading overlay with spinner
static void RenderLoadingOverlay()
{
	const auto& wnd = GetCurrentWindow();
	ImDrawList* dl = wnd->DrawList;
	ImVec2 pos = wnd->Pos;
	ImVec2 size = wnd->Size;

	// Dim background
	dl->AddRectFilled(pos, pos + size, h->CO({ 0, 0, 0, 0.75f }));

	// Spinner
	ImVec2 center = pos + ImVec2(size.x * 0.5f, size.y * 0.5f - 10);
	float t = (float)ImGui::GetTime();
	float radius = 18.0f;
	float start = t * 4.0f;
	float end = start + IM_PI * 1.35f;
	dl->PathClear();
	dl->PathArcTo(center, radius, start, end, 48);
	dl->PathStroke(h->CO(colors::Main), 0, 3.0f);

	// Label
	vec2 ts = h->CT("Signing in...");
	ImVec2 tpos = center + ImVec2(-ts.x * 0.5f, radius + 12);
	dl->AddText(tpos, h->CO(colors::White), "Signing in...");
}

static bool LauncherLogin(const std::string& username, const std::string& password, std::string& outError)
{
    Api::AuthResult res = Api::Login(username, password);
    if (!res.success)
    {
        outError = res.error;
        return false;
    }
    g_token = res.token;
    g_username = res.username;
    g_role = "User";
    g_authenticated = true;

    // Fetch owned products from webapp API
    g_ownedProducts = Api::GetUserLibrary(g_username);
    return true;
}

bool remember;

INT __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // Single-instance guard: prevent multiple instances
    const wchar_t* kSingleInstanceMutexName = L"Global\\OnyxLauncher_SingleInstance";
    HANDLE singleInstanceMutex = CreateMutexW(nullptr, FALSE, kSingleInstanceMutexName);
    if (singleInstanceMutex == nullptr)
        return 0;
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        HWND existing = FindWindowW(L"Window", L"Window");
        if (existing)
        {
            ShowWindow(existing, SW_RESTORE);
            SetForegroundWindow(existing);
        }
        CloseHandle(singleInstanceMutex);
        return 0;
    }

    // if you want to add / remove the window background, navigate to main.hpp and add / remove the define macro ``WINDOW_BG``
    window->Initialize(L"Window"); // creates the whole window, the arg will be the window name
    window->Blur();

    fonts->RenderFonts();

    EIMAGE(window->Dvc, _InputTex, images->inputGlow);
    EIMAGE(window->Dvc, _Google, images->googleIcon);
    EIMAGE(window->Dvc, _Discord, images->discordIcon);
    EIMAGE(window->Dvc, _Logo, images->logo);
    EIMAGE(window->Dvc, _Banner, images->banner);
    EIMAGE(window->Dvc, _Product, images->product);
    EIMAGE(window->Dvc, _DefaultProfilePic, images->profilePic);

    HWND* hRef = &window->hWindow;

    while (window->RenderLoop())
    {
        ImGui::NewFrame();

        custom->Begin("Onyx");
        {
            const auto& window = GetCurrentWindow();

            const static auto& alpha = std::make_unique<Alpha>(); alpha->Render();
            const static auto& subalpha = std::make_unique<Alpha>(); subalpha->Render();

            PushFont(fonts->Icons[0]);

            SetCursorPos({ window->Size.x - h->CT(CLOSE).x - 10, 8 });
            if (items->TextButton(CLOSE, "close_wnd"))
                exit(0);

            SameLine();

            SetCursorPos({ window->Size.x - h->CT(MINUS).x - 10 * 3, 7 });
            if (items->TextButton(MINUS, "minmize_wnd"))
                ShowWindow(*(hRef), SW_MINIMIZE);

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

                    if (alpha->tab == login)
                    {
                        if (strlen(loginErrMsg) > 0)
                        {
                            SetCursorPos({ (window->Size.x - 220) / 2 + 5, 110 });
                            PushFont(fonts->InterM[0]);
                            draw->Text(loginErrMsg, colors::Red);
                            PopFont();
                        }

                        SetCursorPos({ (window->Size.x - 220) / 2, 135 });
                        items->Input("Username", USER, "", loginUsr, _size(loginUsr), 0);

                        SetCursorPosX((window->Size.x - 220) / 2);
                        items->Input("Password", EYE_SLASHED, EYE, loginPas, _size(loginPas), ImGuiInputTextFlags_Password /*THIS FLAG WILL MAKE THE ICON TO CHANGE, MAKE SURE TO SET 2 ICONS*/);
                    }
                    else
                    {
                        SetCursorPos({ (window->Size.x - 220) / 2, 135 });
                        items->Input("Username", USER, "", registerUsr, _size(registerUsr), 0);

                        SetCursorPosX((window->Size.x - 220) / 2);
                        items->Input("Password", EYE_SLASHED, EYE, registerPas, _size(registerPas), ImGuiInputTextFlags_Password /*THIS FLAG WILL MAKE THE ICON TO CHANGE, MAKE SURE TO SET 2 ICONS*/);
                    }

                    if (alpha->tab == login)
                    {
                        SetCursorPosX((window->Size.x - 215) / 2);
                        items->Checkbox("Remember me", &remember);

                        // While authenticating, do NOT show spinner yet

                        // Transition or show error when async login completes
                        if (g_isLoading && g_loginState == 2)
                        {
                            // Successful login: show a 3s spinner on login screen, then navigate
                            g_showPostLoginSpinner = true;
                            g_postSpinnerEndTime = ImGui::GetTime() + kPostLoginSpinnerSeconds;
                            g_isLoading = false;
                            g_loginState = 0;
                            // Navigation will occur after the spinner delay (handled below)
                        }
                        else if (g_isLoading && g_loginState == 3)
                        {
                            // Failure: no spinner, just show error
                            g_isLoading = false;
                            g_loginState = 0;
                            strncpy_s(loginErrMsg, sizeof(loginErrMsg), g_loginError.c_str(), _TRUNCATE);
                            loginErrMsg[sizeof(loginErrMsg) - 1] = '\0';
                        }
                    }

                    if (alpha->tab == registerr)
                    {
                        SetCursorPosX((window->Size.x - 220) / 2);
                        items->Input("License key", KEY, "", licbuf, _size(licbuf));
                    }

                    std::string buttonLabel = (alpha->tab == login) ? "SIGN IN" : "SIGN UP";
                    SetCursorPosX((window->Size.x - 220) / 2);
                    bool _clickedSubmit = items->Button(buttonLabel, { window->Size.x - GetCursorPosX() * 2, 40 });
                    bool _enterPressed = IsKeyPressed(ImGuiKey_Enter) || IsKeyPressed(ImGuiKey_KeypadEnter);
                    if (_clickedSubmit || _enterPressed)
                    {
                        if (alpha->tab == login)
                        {
                            bool _userEmpty = strlen(loginUsr) == 0;
                            bool _passEmpty = strlen(loginPas) == 0;
                            items->SetInputError("Username", _userEmpty);
                            items->SetInputError("Password", _passEmpty);
                            if (!_userEmpty && !_passEmpty)
                            {
                                // Start async login with loading overlay
                                g_isLoading = true;
                                g_loginState = 1;
                                loginErrMsg[0] = '\0';
                                std::thread([=]() {
                                    std::string err;
                                    bool ok = LauncherLogin(loginUsr, loginPas, err);
                                    if (ok)
                                    {
                                        g_loginState = 2;
                                    }
                                    else
                                    {
                                        g_loginError = err;
                                        g_loginState = 3;
                                    }
                                }).detach();
                            }
                        }
                        else
                        {
                            bool _userEmpty = strlen(registerUsr) == 0;
                            bool _passEmpty = strlen(registerPas) == 0;
                            bool _licEmpty = strlen(licbuf) == 0;
                            items->SetInputError("Username", _userEmpty);
                            items->SetInputError("Password", _passEmpty);
                            items->SetInputError("License key", _licEmpty);
                            if (!_userEmpty && !_passEmpty && !_licEmpty)
                            {
                                // After successful sign up, redirect to Sign in page
                                alpha->index = login;
                            }
                        }
                    }

                    PopFont();

                    PushFont(fonts->InterM[0]);

                    std::string textLabel = (alpha->tab == login) ? "Don't you have an account? " : "Already have an account? ";
                    std::string redirectLabel = (alpha->tab == login) ? "Sign up" : "Sign in";

                    SetCursorPosX((window->Size.x - h->CT(textLabel + redirectLabel).x) / 2);
                    draw->Text(textLabel, colors::Lwhite2);

                    SameLine();
                    if (items->TextButton(redirectLabel, redirectLabel, colors::Main))
                    {
                        // Clear all auth-related buffers when switching forms
                        ZeroMemory(loginUsr, sizeof(loginUsr));
                        ZeroMemory(loginPas, sizeof(loginPas));
                        ZeroMemory(registerUsr, sizeof(registerUsr));
                        ZeroMemory(registerPas, sizeof(registerPas));
                        ZeroMemory(licbuf, sizeof(licbuf));

                        if (alpha->index == login)
                            alpha->index = registerr;
                        else
                            alpha->index = login;
                    }

                    PopFont();

                    if (alpha->tab == login)
                    {
                        PushFont(fonts->InterM[1]);

                        SetCursorPosX((window->Size.x - h->CT("or").x) / 2);
                        draw->Text("or", colors::Lwhite2);

                        float lenght = 65; 
                        float distance = 15; 
                        float centerY = (GetCursorPosY() - GetStyle().ItemSpacing.y * 2) + h->CT("or").y * 0.5f; 

                        window->DrawList->AddLine(window->Pos + vec2(window->Size.x * 0.5f - h->CT("or").x * 0.5f - lenght - distance, centerY), window->Pos + vec2(window->Size.x * 0.5f - h->CT("or").x * 0.5f - distance, centerY), h->CO(colors::Gray)); 
                        window->DrawList->AddLine(window->Pos + vec2(window->Size.x * 0.5f + h->CT("or").x * 0.5f + distance, centerY), window->Pos + vec2(window->Size.x * 0.5f + h->CT("or").x * 0.5f + lenght + distance, centerY), h->CO(colors::Gray));

                        SetCursorPos({(window->Size.x * 0.5f - h->CT("or").x * 0.5f - lenght - distance + window->Size.x * 0.5f - h->CT("or").x * 0.5f - distance) * 0.5f, GetCursorPosY() - 5});
                        if (items->ImageButton("google", images->googleIcon, { 20, 20 }))
                        {

                        }

                        SameLine();

                        SetCursorPosX((window->Size.x / 2 + h->CT("or").x / 2 + distance + lenght / 2) - 44);
                        if (items->ImageButton("discord", images->discordIcon, { 21, 15 }))
                        {
                            std::thread([&] {
                                ShellExecuteW(nullptr, L"open", L"https://discord.gg/7bcTMxvmAU", nullptr, nullptr, SW_SHOWNORMAL);
                            }).detach();
                        }

                        PopFont();

                        // After success: show spinner as transition on login view, then navigate
                        if (g_showPostLoginSpinner)
                        {
                            RenderLoadingOverlay();
                            if (ImGui::GetTime() >= g_postSpinnerEndTime)
                            {
                                g_showPostLoginSpinner = false;
                                alpha->index = home;
                                subalpha->index = dashboard;
                            }
                        }
                    }
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
                    const char* dispUser = g_authenticated ? g_username.c_str() : "relique";
                    std::string roleDisplay = g_authenticated ? FormatRole(g_role) : std::string("User");
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
                    // If we have a pending post-login spinner, render it and delay entering dashboard for 3 seconds
                    if (g_showPostLoginSpinner)
                    {
                        RenderLoadingOverlay();
                        if (ImGui::GetTime() >= g_postSpinnerEndTime)
                        {
                            g_showPostLoginSpinner = false;
                        }
                        else
                        {
                            // Skip rendering dashboard contents while spinner is shown
                            goto SKIP_DASHBOARD_CONTENTS;
                        }
                    }
                    SetCursorPos({ 190, 33 });
                    custom->Banner("Dashboard", "Welcome back, relique", images->banner);

                    SetCursorPos({ 190, 230 });
                    custom->BeginChild("News", { window->Size.x + 160 - GetCursorPosX() * 2, (window->Size.y - 200 - 50) - 10/*change this if you wanna change height*/});
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

                    // Render owned products from g_ownedProducts; show empty state otherwise
                    int column = 0;
                    float startX = 190.0f;
                    SetCursorPos({ startX, 70 });
                    if (!g_ownedProducts.empty())
                    {
                        for (size_t i = 0; i < g_ownedProducts.size(); ++i)
                        {
                            const auto& p = g_ownedProducts[i];
                            ProductStatus status = ProductStatus::Online;
                            if (p.status == "offline") status = ProductStatus::Offline;
                            else if (p.status == "updating") status = ProductStatus::Updating;

                            // Compute precise time-left from ISO expiry
                            vec4 timeLeftColor = colors::Main;
                            std::string timeLeftText = FormatTimeLeftFromIso(p.expiresAt, timeLeftColor);

                            if (items->Product(p.name, timeLeftText, status, images->product, timeLeftColor))
                            {
                                // TODO: handle product click (launch, details, etc.)
                            }

                            // layout: 3 per row
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
                            float rounding = avatarSize.y * 0.5f; // circle
                            child->DrawList->AddImageRounded((ImTextureID)images->profilePic, avatarPos, avatarPos + avatarSize, {}, { 1, 1 }, h->CO(colors::White), rounding);
                            child->DrawList->AddRect(avatarPos, avatarPos + avatarSize, h->CO(colors::Gray), rounding, 0, 1.0f); // light border
                        }

                        PushFont(fonts->InterM[2]);

                        SetCursorPos({ 95, 30 });
                        draw->Text(g_authenticated ? g_username.c_str() : "relique", colors::Main);

                        SetCursorPos({ 95, 50 });
                        PushFont(fonts->userUidFont);
                        draw->Text("UID: Unknown", colors::Lwhite2);
                        PopFont();

                        {
                            std::string role = g_authenticated ? FormatRole(g_role) : std::string("User");
                            PushFont(fonts->profileRoleFont);
                            vec2 roleSize = h->CT(role);

                            float rightPadding = 14; // move left by increasing padding
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
        custom->End();

        window->CleanImGui();
    }
    window->CleanWindow();
    CloseHandle(singleInstanceMutex);
    return 0;
}