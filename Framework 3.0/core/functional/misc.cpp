#include "colors.hpp"
#include "data/fonts.h"

void Gui::SmoothResize(RECT rc)
{
    int Width = static_cast<int>(window->size.x);
    int Height = static_cast<int>(window->size.y);

    int CenterX = rc.left + (rc.right - rc.left) / 2;
    int CenterY = rc.top + (rc.bottom - rc.top) / 2;

    int NewX = CenterX - Width / 2;
    int NewY = CenterY - Height / 2;

    MoveWindow(window->hWindow, NewX, NewY, Width, Height, true);
}

void Gui::MoveWindowW()
{
    RECT rc, winRect;
    ImGuiIO& io = GetIO();

    GetWindowRect(window->hWindow, &rc);
    MoveWindow(window->hWindow, rc.left + GetWindowPos().x, rc.top + GetWindowPos().y, (window->size.x), (window->size.y), TRUE);
    gui->Size = ImLerp(gui->Size, gui->SizeOnChange, vtime(18));

    GetWindowRect(window->hWindow, &winRect);
    SmoothResize(winRect);

    SetWindowPos(ImVec2(0.f, 0.f));
}

void Gui::Style()
{
    ImGuiStyle& style = GetStyle();

    style.WindowBorderSize = 0.f;
    style.ScrollbarSize = 0.f;
    style.WindowPadding = { 0.f, 0.f };
    style.WindowShadowSize = 0.f;
    style.Colors[ImGuiCol_ChildBg] = colors::Transparent;
    style.Colors[ImGuiCol_TextSelectedBg] = colors::Second;
}

static void SetRoundedCorners(HWND hwnd)
{
    DWORD preference = 2;
    DwmSetWindowAttribute(hwnd, 33, &preference, sizeof(preference));
}

VOID Window::Blur()
{
    SetRoundedCorners(this->hWindow); //only for windows 11

    struct ACCENTPOLICY
    {
        int na;
        int nf;
        int nc;
        int nA;
    };
    struct WINCOMPATTRDATA
    {
        int na;
        PVOID pd;
        ULONG ul;
    };

    const HINSTANCE hm = LoadLibrary(L"user32.dll");
    if (hm)
    {
        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINCOMPATTRDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = (pSetWindowCompositionAttribute)GetProcAddress(hm, "SetWindowCompositionAttribute");
        if (SetWindowCompositionAttribute)
        {
            ACCENTPOLICY policy = { 3, 0, 0, 0 }; // 4,0,155,0 (Acrylic blur) //  3,0,0,0 
            WINCOMPATTRDATA data = { 19, &policy,sizeof(ACCENTPOLICY) };
            SetWindowCompositionAttribute(this->hWindow, &data);
        }
        FreeLibrary(hm);
    }
}

bool Fonts::RenderFonts()
{
    ImGuiIO& io = GetIO();
    ImFontConfig cfg;
    cfg.FontDataOwnedByAtlas = false;
    cfg.FontBuilderFlags = ImGuiFreeTypeBuilderFlags_ForceAutoHint | ImGuiFreeTypeBuilderFlags_LightHinting | ImGuiFreeTypeBuilderFlags_LoadColor;

    for (int i = 0; i < InterMSizes.size(); ++i)
        InterM[i] = EFONT(_InterM, InterMSizes[i] + 3, cfg);

    for (int i = 0; i < InterSSizes.size(); ++i)
        InterS[i] = EFONT(_InterS, InterSSizes[i] + 3, cfg);

    for (int i = 0; i < IconSSizes.size(); ++i)
        Icons[i] = EFONT(_Icons, IconSSizes[i], cfg);

    Pass = EFONT(_Pass, 25, cfg);
    Xirod = EFONT(_Xirod, 30, cfg);
    RennerM = EFONT(_RennerM, 16, cfg);

    return true;
}

bool Alpha::Render()
{
    if (alpha == 0)
    {
        last = tab;
        tab = index;
    }

    time = gui->vtime(5);
    return alpha = std::clamp(alpha + (time * (index == tab ? 1.f : -1.f)), 0.f, 1.f);
}

int Alpha::GetLastTab()
{
    return last;
}

