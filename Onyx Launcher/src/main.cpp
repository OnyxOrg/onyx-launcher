#include "includes\\core\\items\\custom.hpp"
#include "includes\\core\\app.hpp"
#include "includes\\core\\app_state.hpp"
#include <thread>
#include <ctime>
#include <chrono>
#include <cstdio>
#include <atomic>
#include "includes\\api\\auth.hpp"
#include "includes\\api\\library.hpp"
#include <cstring>

static AppState g_state;

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

    // Fetch owned products from webapp API.
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
        App::RenderFrame(g_state);
        custom->End();
        window->CleanImGui();
    }
    window->CleanWindow();
    CloseHandle(singleInstanceMutex);
    return 0;
}