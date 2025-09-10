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