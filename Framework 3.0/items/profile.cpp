#include "custom.hpp"

namespace profilecol
{
	vec4 namec = colors::Main;
	vec4 rolec = colors::RoleMember;
}

struct Profiles
{
    vec4 bg;
    vec2 p;
    float time;
};

bool Items::Profile(const std::string& name, const std::string& role, ID3D11ShaderResourceView* tex)
{
	const auto& window = GetCurrentWindow();
	using namespace profilecol;

    vec2 pos = window->DC.CursorPos;
    vec2 size = { 130, 45 };

    bool r = InvisibleButton(name.c_str(), size);
    bool hov = IsItemHovered();

    static std::map<std::string, Profiles> anim;
    auto& it = anim.emplace(name, Profiles()).first->second;

    vec4 bgcol = hov ? colors::ItemBg : colors::Transparent;

    it.time = gui->xtime(gui->GlobalSpeed);
    it.p.x = ImLerp(it.p.x, hov ? pos.x + 5.f : pos.x, it.time);
    it.p.y = pos.y;
    it.bg = ImLerp(it.bg, bgcol, it.time);

    window->DrawList->AddRectFilled(it.p, it.p + size, h->CO(it.bg), 8);
    window->DrawList->AddImageRounded((ImTextureID)tex, it.p + vec2(5, size.y * 0.5f - 35 * 0.5f), it.p + vec2(40, size.y * 0.5f + 35 * 0.5f), {}, { 1, 1 }, h->CO(colors::White), 1000);
    // Username uses whatever font is currently pushed by caller
    window->DrawList->AddText(it.p + vec2(50, size.y * 0.5f - 15), h->CO(namec), name.c_str());

    // Role uses independent font and size
    PushFont(fonts->userRoleFont);
    window->DrawList->AddText(it.p + vec2(50, size.y * 0.5f + 2), h->CO(rolec), role.c_str());
    PopFont();

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	return r;
}