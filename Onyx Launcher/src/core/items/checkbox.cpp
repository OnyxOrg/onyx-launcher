#include "includes/core/items/custom.hpp"
#include "includes/core/items/overlay.hpp"

namespace checkboxcol {

	vec4 textC = colors::Lwhite2;
	vec4 textAct = colors::Lwhite2;

	vec4 bg = { colors::Main.x, colors::Main.y, colors::Main.z, 0.2};
	vec4 bgHov = { colors::Main.x, colors::Main.y, colors::Main.z, 0.4 };
	vec4 bgAct = colors::Main;

	vec4 circle = colors::Second;
	vec4 circleAct = colors::Lwhite;

	vec4 border = { colors::Main.x, colors::Main.y, colors::Main.z, 0.2 };
}

struct Checkboxs
{
	vec4 bg, text, circle, border;
	float x, time;
};

bool Items::Checkbox(const std::string& label, bool* v) 
{
	const auto& window = GetCurrentWindow();
	using namespace checkboxcol;

	vec2 pos = window->DC.CursorPos;
	vec2 size = { window->Size.x - GetCursorPosX() * 2, 16};

	vec2 csize = { 26, 16 };
	vec2 labelSize = h->CT(label.c_str());

	bool r = InvisibleButton(label.c_str(), size);
	bool act(*v); bool hov = IsItemHovered();
	bool chov = IsMouseHoveringRect(pos + vec2(size.x - csize.x, 0), pos + vec2(size.x, csize.y));
	bool overlayBlocks = IsOverlayActive();

	if (!overlayBlocks && r && chov) *v = !*v;

	if (chov && !overlayBlocks)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	vec4 textcol = act ? textAct : textC;
	vec4 bgcol = act ? bgAct : (overlayBlocks ? bg : (hov ? bgHov : bg));
	vec4 circlecol = act ? circleAct : circle;
	vec4 bordercol = act ? colors::Transparent : border;
	float x = act ? /*pos.x + */size.x - 8 : /*pos.x + */size.x - csize.x + 8;

	static std::map<std::string, Checkboxs> anim;
	auto& w = anim.emplace(label, Checkboxs(bgcol, textcol, circlecol, bordercol, x)).first->second;

	w.time = gui->xtime(gui->GlobalSpeed);
	w.text = ImLerp(w.text, textcol, w.time);
	w.bg = ImLerp(w.bg, bgcol, w.time);
	w.border = ImLerp(w.border, bordercol, w.time);
	w.circle = ImLerp(w.circle, circlecol, w.time);
	w.x = ImLerp(w.x, x, w.time * 2);

	window->DrawList->AddText(pos + vec2(0, (size.y - labelSize.y) / 2), h->CO(w.text), label.c_str());
	window->DrawList->AddRectFilled(pos + vec2(size.x - csize.x, 0), pos + vec2(size.x, csize.y), h->CO(w.bg), 100);
	window->DrawList->AddCircleFilled(pos + vec2(w.x, csize.y / 2), 5, h->CO(w.circle), 1000);
	window->DrawList->AddRect(pos + vec2(size.x - csize.x, 0), pos + vec2(size.x, csize.y), h->CO(w.border), 1000);

	return r;
}


