#include "custom.hpp"

namespace tabcol
{
	vec4 textC = colors::Lwhite2;
	vec4 textAct = colors::Lwhite;

	vec4 bgC = colors::Transparent;
	vec4 bgAct = colors::ItemBg;

	float rounding = 5;
}

struct Tabs
{
	vec4 col, bg;
	float time;
};

bool Items::Tab(const std::string& label, const std::string& icon, bool cond)
{
	const auto& window = GetCurrentWindow();
	using namespace tabcol;

	vec2 pos = window->DC.CursorPos;
	vec2 labelSize = h->CT(label);

	PushFont(fonts->Icons[2]);
	vec2 iconSize = h->CT(icon);
	PopFont();

	float gap = iconSize.x + 15;

	vec2 size = vec2(130, 35);

	bool r = InvisibleButton(label.c_str(), size);
	bool act = cond; bool hov = IsItemHovered();

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	vec4 col = act ? textAct : textC;
	vec4 bgcol = act ? bgAct : bgC;

	static std::map<std::string, Tabs> anim;
	auto& it = anim.emplace(label, Tabs(col, bgcol)).first->second;

	it.time = gui->xtime(gui->GlobalSpeed);
	it.col = ImLerp(it.col, col, it.time);
	it.bg = ImLerp(it.bg, bgcol, it.time);

	window->DrawList->AddRectFilled(pos, pos + size, h->CO(it.bg), rounding);

	PushFont(fonts->Icons[2]),
	window->DrawList->AddText(pos + vec2(5, (size.y - labelSize.y) / 2), h->CO(it.col), icon.c_str());
	PopFont();

	window->DrawList->AddText(pos + vec2(gap, (size.y - labelSize.y) / 2), h->CO(it.col), label.c_str());

	return r;
}