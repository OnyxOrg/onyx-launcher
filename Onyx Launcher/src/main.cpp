#include "includes\\core\\items\\custom.hpp"
#include "includes\\core\\app.hpp"
#include "includes\\core\\app_state.hpp"

static AppState g_state;


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