#include "includes/core/items/custom.hpp"

// Error state per input name
static std::map<std::string, bool> g_inputError;

namespace inputcol
{
	vec4 bg = colors::ItemBg;
	vec4 bgAct = bg;

	vec4 textC = colors::Lwhite2;
	vec4 textAct = colors::Lwhite;

	vec4 iconC = colors::Gray;
	vec4 iconAct = colors::Lwhite2;

	vec4 shadowAct = colors::Main;
	vec4 shadowC = colors::Transparent;

	vec4 border = colors::Gray;

	float rounding = 10;
}

struct Inputs
{
	vec4 bg, text, icon, shadow;
	std::string currentIcon;
	bool changeIcon;
	float time;
};

bool Items::Input(const std::string& name, const std::string& iconA, const std::string& iconB, char* buf, size_t bufSize, ImGuiInputTextFlags flags)
{
	const auto& window = GetCurrentWindow();
	using namespace inputcol;

	vec2 pos = window->DC.CursorPos;
	vec2 size = { window->Size.x - GetCursorPosX() * 2, 13 };

	static std::map<std::string, Inputs> anim;
	auto& w = anim.emplace(name, Inputs(bg, textC, iconC, shadowC, iconA)).first->second;

	SetNextItemWidth(size.x);
	PushStyleVar(ImGuiStyleVar_FramePadding, { 12, size.y });
	PushStyleColor(ImGuiCol_Text, textAct);
	PushStyleColor(ImGuiCol_FrameBg, colors::Transparent);

	window->DrawList->AddRectFilled(pos, pos + vec2(size.x, GetFrameHeight()), h->CO(w.bg), rounding);
	bool _hasText = strlen(buf) > 0;
	if (_hasText && g_inputError[name])
		g_inputError[name] = false;
	vec4 _borderCol = (g_inputError[name] && !_hasText) ? colors::Red : border;
	window->DrawList->AddRect(pos, pos + vec2(size.x, GetFrameHeight()), h->CO(_borderCol), rounding);

	//should be done with shadow rect, but it doesnt support that ammount of rounding in a short radius (could be done if it the item hasn't had transparent background tho)
	//window->DrawList->AddShadowRect(pos, pos + vec2(size.x, GetFrameHeight()), h->CO({colors::Main.x, colors::Main.y, colors::Main.z, 0.5}), 15, {0, 0}, ImDrawFlags_ShadowCutOutShapeBackground | ImDrawFlags_RoundCornersMask_, 10000);
	window->DrawList->AddImage((ImTextureID)images->inputGlow, pos - vec2(3, 3), pos + vec2(size.x, GetFrameHeight()) + vec2(3, 3), {}, { 1, 1 }, h->CO(w.shadow));

	uint64_t uFlags = uFlags = w.changeIcon ? flags & ~(ImGuiInputTextFlags_Password) : flags;
	std::string inputLabel = "###" + name;

	window->DrawList->PushClipRect(pos, pos + vec2(size.x - (12 * 2) - h->CT(iconA).x, GetFrameHeight()));
	bool r = InputText(inputLabel.c_str(), buf, bufSize, uFlags);
	window->DrawList->PopClipRect();

	bool act(IsItemActive());
	bool hovCond = IsMouseHoveringRect(pos + vec2(size.x - (12 * 2) - h->CT(w.currentIcon).x, 0), pos + vec2(size.x, GetFrameHeight()));

	vec4 textcol = act || strlen(buf) > 0 ? colors::Transparent : textC;
	vec4 iconcol = (hovCond && (flags & ImGuiInputTextFlags_Password)) || act ? iconAct : iconC;
	vec4 bgcol = act ? bgAct : bg;
	vec4 shadowcol = act ? shadowAct : shadowC;

	vec2 nameSize = h->CT(name.c_str());

	w.time = gui->xtime(gui->GlobalSpeed);
	w.text = ImLerp(w.text, textcol, w.time);
	w.bg = ImLerp(w.bg, bgcol, w.time);
	w.icon = ImLerp(w.icon, iconcol, w.time);
	w.shadow = ImLerp(w.shadow, shadowcol, w.time);

	window->DrawList->AddText(pos + vec2(GetStyle().FramePadding.x, (GetFrameHeight() - nameSize.y) / 2), h->CO(w.text), name.c_str());

	PushFont(fonts->Icons[1]);
	bool changeIconCond = hovCond && IsMouseClicked(0) && (flags & ImGuiInputTextFlags_Password);

	if (changeIconCond)
		w.changeIcon = !w.changeIcon;

	if (hovCond && (flags & ImGuiInputTextFlags_Password))
		SetMouseCursor(ImGuiMouseCursor_Hand);

	w.currentIcon = w.changeIcon ? iconB : iconA;
	window->DrawList->AddText(pos + vec2(size.x - h->CT(w.currentIcon).x - 12, (GetFrameHeight() - h->CT(w.currentIcon).y) / 2) - vec2(0, 1.5), h->CO(w.icon), w.currentIcon.c_str());
	PopFont();

	PopStyleColor(2);
	PopStyleVar();

	return r;
}

void Items::SetInputError(const std::string& name, bool error)
{
	g_inputError[name] = error;
}

void Items::ClearInputError(const std::string& name)
{
	g_inputError[name] = false;
}


