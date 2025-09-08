#include ".\items\custom.hpp"
#include <thread>

char loginUsr[42]
    ,loginPas[42]
    ,registerUsr[42]
    ,registerPas[42]
    ,licbuf[42]; // + 1 byte for null character

bool remember;

INT __stdcall WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    // Single-instance guard: prevent multiple instances
    const wchar_t* kSingleInstanceMutexName = L"Global\\OnyxFramework3_SingleInstance";
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
    EIMAGE(window->Dvc, _ProfilePic, images->profilePic);

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

            SetCursorPos({ window->Size.x - h->CT(CLOSE).x - 10, 5 });
            if (items->TextButton(CLOSE, "close_wnd"))
                exit(0);

            SameLine();

            SetCursorPosX(window->Size.x - h->CT(MINUS).x - 10 * 3);
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
                                alpha->index = home;
                                subalpha->index = dashboard; // make sure to change this aswell
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
                if (items->Profile("relique", "User", images->profilePic)) subalpha->index = profile;
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
                    SetCursorPos({ 190, 27 });
                    custom->Banner("Dashboard", "Welcome back, relique", images->banner);

                    SetCursorPos({ 190, 200 });
                    custom->BeginChild("News", { window->Size.x + 160 - GetCursorPosX() * 2, (window->Size.y - 200 - 50) - 10/*change this if you wanna change height*/});
                    {
                        PushStyleVarY(ImGuiStyleVar_ItemSpacing, 12);

                        items->Announcement("ONYX UPDATE 1.0", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", feature);
                        items->Announcement("ONYX UPDATE 0.4", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", updated);
                        items->Announcement("ONYX UPDATE 0.1", "Lorem ipsum dolor sit amet consectetur adipisicing elit. Possimus, perferendis\nipsum? Itaque, ea harum. Aliquam libero animi maxime ab, sapiente beatae maiores\nobcaecati quae. Modi officiis dolore delectus ullam rem?", "August 6, 2025", improvement);
                        
                        Spacing();
                        PopStyleVar(); // itemspacing
                    }
                    custom->EndChild();
                }

                if (subalpha->tab == library)
                {
                    PushFont(fonts->InterS[1]);

                    SetCursorPos({ 190, 25 });
                    draw->Text("LIBRARY", colors::White);

                    PopFont();

                    PushStyleVar(ImGuiStyleVar_ItemSpacing, { 20, 20 });

                    SetCursorPos({ 190, 70 });
                    if (items->Product("Aura", "Never", ProductStatus::Online, images->product))
                    {

                    }

                    SameLine();
                    if (items->Product("Supernova", "3 Days", ProductStatus::Updating, images->product))
                    {

                    }

                    SameLine();
                    if (items->Product("Another one", "1 Day", ProductStatus::Offline, images->product))
                    {

                    }

                    // if you want to add more simply adjust x-pos (SetCursorPosX)
                    /*
                    * SetCursorPosX(190);
                    * if (items->product)...
                    * 
                    * SameLine();
                    * if (items->product)...
                    */

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
                        draw->Text("relique", colors::Main);

                        SetCursorPos({ 95, 50 });
                        draw->Text("UID: Unknown", colors::Lwhite2);

                        {
                            std::string role = "User";
                            vec2 roleSize = h->CT(role);

                            child->DrawList->AddRectFilled({ child->Pos.x + child->Size.x - (15 + roleSize.x) - 12, child->Pos.y + (child->Size.y - 25) / 2 }, { child->Pos.x + child->Size.x - 12, child->Pos.y + (child->Size.y - 25) / 2 + 25 }, h->CO(colors::Gray2), 8);
                            child->DrawList->AddText({ child->Pos.x + child->Size.x - (15 + roleSize.x) - 12 + (15 + roleSize.x - roleSize.x) / 2, child->Pos.y + (child->Size.y - 25) / 2 + (25 - roleSize.y) / 2 }, h->CO(colors::Lwhite2), role.c_str());
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