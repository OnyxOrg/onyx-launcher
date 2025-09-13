#include "includes/core/items/custom.hpp"

namespace buttoncol
{
	vec4 bg = colors::ItemBg;
	vec4 bgAct = colors::Main;

	vec4 textC = colors::Lwhite2;
	vec4 textAct = colors::Lwhite;

	float rounding = 7;
}

struct Buttons
{
	vec4 bgcol, textcol;
	float time;
};

bool Items::Button(const std::string& label, const vec2& size)
{
	ImGuiWindow* window = GetCurrentWindow();
	using namespace buttoncol;

	vec2 lSize(h->CT(label.c_str()));
	vec2 pos = window->DC.CursorPos;

	static std::map<std::string, Buttons> anim;

	bool r = InvisibleButton(label.c_str(), size);
	bool act(IsItemActive()); bool hov(IsItemHovered());

	vec4 bgcol = act ? bgAct : bg;
	vec4 textcol = act ? textAct : textC;

	Buttons& w = anim.emplace(label, Buttons(bgcol, textcol)).first->second;

	w.time = gui->xtime(gui->GlobalSpeed);
	w.bgcol = ImLerp(w.bgcol, bgcol, w.time);
	w.textcol = ImLerp(w.textcol, textcol, w.time);

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	window->DrawList->AddRectFilled(pos, pos + size, h->CO(w.bgcol), rounding);
	window->DrawList->AddText(pos + (size - lSize) / 2, h->CO(w.textcol), label.c_str());

	return r;
}

bool Items::TextButton(const std::string& text, const std::string& id, vec4 col)
{
	const auto& window = GetCurrentWindow();

	vec2 pos = window->DC.CursorPos;
	vec2 size = h->CT(text);

	bool r = InvisibleButton(id.c_str(), size);
	bool act = IsItemActive(); bool hov = IsItemHovered();

	vec4 textColor = act ? colors::Lwhite : hov ? colors::Lwhite : colors::Lwhite2;

	static std::map<std::string, Buttons> anim;
	auto& it = anim.emplace(id, Buttons(vec4(), textColor)).first->second;

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	it.time = gui->xtime(gui->GlobalSpeed);
	it.textcol = ImLerp(it.textcol, textColor, it.time);

	if (!col.x)
		window->DrawList->AddText(pos, h->CO(it.textcol), text.c_str());
	else
		window->DrawList->AddText(pos, h->CO(col), text.c_str());

	return r;
}

bool Items::ImageButton(const std::string& label, ID3D11ShaderResourceView* image, vec2 imgSize)
{
	const auto& window = GetCurrentWindow();
	using namespace buttoncol;

	vec2 pos = window->DC.CursorPos;
	vec2 buttonSize = { 44, 44 };

	static std::map<std::string, Buttons> anim;
	auto& it = anim.emplace(label, Buttons()).first->second;

	bool r = InvisibleButton(label.c_str(), buttonSize);
	bool hov = IsItemHovered();

	vec4 bgcol = hov ? vec4(colors::ItemBg.x, colors::ItemBg.y, colors::ItemBg.z, 0.4) : colors::ItemBg;

	it.time = gui->xtime(gui->GlobalSpeed);
	it.bgcol = ImLerp(it.bgcol, bgcol, it.time);

	window->DrawList->AddRectFilled(pos, pos + buttonSize, h->CO(it.bgcol), 5);
	window->DrawList->AddImage((ImTextureID)image, pos + (buttonSize - imgSize) * 0.5f, pos + (buttonSize + imgSize) * 0.5f, {}, { 1, 1 }, h->CO(colors::White));

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	return r;
}

namespace disccordbutton
{
	vec4 bg = rgba(80, 101, 242);
	vec4 bgAct = rgba(65, 76, 192);

	vec4 textC = colors::White;
	vec4 textAct = colors::White;

	float rounding = 7;
}

bool Items::ButtonIcon(const std::string& label, const std::string& icon, const vec2& size)
{
	ImGuiWindow* window = GetCurrentWindow();
	using namespace disccordbutton;

	vec2 lSize(h->CT(label.c_str()));
	vec2 pos = window->DC.CursorPos;

	PushFont(fonts->Icons[4]);
	vec2 iSize = h->CT(icon);
	PopFont();

	static std::map<std::string, Buttons> anim;

	bool r = InvisibleButton(label.c_str(), size);
	bool act(IsItemActive()); bool hov(IsItemHovered());

	vec4 bgcol = hov ? bgAct : bg;
	vec4 textcol = hov ? textAct : textC;

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	Buttons& w = anim.emplace(label, Buttons(bgcol, textcol)).first->second;

	w.time = gui->xtime(gui->GlobalSpeed);
	w.bgcol = ImLerp(w.bgcol, bgcol, w.time);
	w.textcol = ImLerp(w.textcol, textcol, w.time);

	window->DrawList->AddRectFilled(pos, pos + size, h->CO(w.bgcol), rounding);

	vec2 combinedSize = lSize + iSize;
	float padding = 2;

	window->DrawList->AddText(pos + (size - vec2(combinedSize.x, lSize.y)) / 2 + vec2(iSize.x + padding, 0), h->CO(w.textcol), label.c_str());

	PushFont(fonts->Icons[4]);
	window->DrawList->AddText(
		pos + (size - vec2(combinedSize.x, iSize.y)) / 2 - vec2(padding, -1),
		h->CO(w.textcol),
		icon.c_str()
	);
	PopFont();

	return r;
}

namespace dangerbutton
{
    vec4 bg = rgba(227, 72, 80);     // red
    vec4 bgAct = rgba(192, 52, 58);  // darker red

    vec4 textC = colors::White;
    vec4 textAct = colors::White;

    float rounding = 7;
}

bool Items::ButtonDangerIcon(const std::string& label, const std::string& icon, const vec2& size)
{
    ImGuiWindow* window = GetCurrentWindow();
    using namespace dangerbutton;

    vec2 lSize(h->CT(label.c_str()));
    vec2 pos = window->DC.CursorPos;

    PushFont(fonts->Icons[4]);
    vec2 iSize = h->CT(icon);
    PopFont();

    static std::map<std::string, Buttons> anim;

    bool r = InvisibleButton(label.c_str(), size);
    bool act(IsItemActive()); bool hov(IsItemHovered());

    vec4 bgcol = hov ? bgAct : bg;
    vec4 textcol = hov ? textAct : textC;

    if (hov)
        SetMouseCursor(ImGuiMouseCursor_Hand);

    Buttons& w = anim.emplace(label, Buttons(bgcol, textcol)).first->second;

    w.time = gui->xtime(gui->GlobalSpeed);
    w.bgcol = ImLerp(w.bgcol, bgcol, w.time);
    w.textcol = ImLerp(w.textcol, textcol, w.time);

    window->DrawList->AddRectFilled(pos, pos + size, h->CO(w.bgcol), rounding);

    vec2 combinedSize = lSize + iSize;
    float padding = 2;

    window->DrawList->AddText(pos + (size - vec2(combinedSize.x, lSize.y)) / 2 + vec2(iSize.x + padding, 0), h->CO(w.textcol), label.c_str());

    PushFont(fonts->Icons[4]);
    window->DrawList->AddText(
        pos + (size - vec2(combinedSize.x, iSize.y)) / 2 - vec2(padding, -1),
        h->CO(w.textcol),
        icon.c_str()
    );
    PopFont();

    return r;
}


bool Items::ButtonDanger(const std::string& label, const vec2& size)
{
    ImGuiWindow* window = GetCurrentWindow();
    using namespace dangerbutton;

    vec2 lSize(h->CT(label.c_str()));
    vec2 pos = window->DC.CursorPos;

    static std::map<std::string, Buttons> anim;

    bool r = InvisibleButton(label.c_str(), size);
    bool act(IsItemActive()); bool hov(IsItemHovered());

    vec4 bgcol = hov ? bgAct : bg;
    vec4 textcol = hov ? textAct : textC;

    if (hov)
        SetMouseCursor(ImGuiMouseCursor_Hand);

    Buttons& w = anim.emplace(label, Buttons(bgcol, textcol)).first->second;

    w.time = gui->xtime(gui->GlobalSpeed);
    w.bgcol = ImLerp(w.bgcol, bgcol, w.time);
    w.textcol = ImLerp(w.textcol, textcol, w.time);

    window->DrawList->AddRectFilled(pos, pos + size, h->CO(w.bgcol), rounding);
    window->DrawList->AddText(pos + (size - lSize) / 2, h->CO(w.textcol), label.c_str());

    return r;
}


