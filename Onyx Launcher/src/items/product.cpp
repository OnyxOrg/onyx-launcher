#include "../includes/items/custom.hpp"
#include <math.h>

namespace productcol
{
	vec4 fadeC = vec4(0, 0, 0, 0.9);
	vec4 fadeHov = vec4(0, 0, 0, 0.4);

	vec4 iconC = colors::Lwhite2;
	vec4 iconHov = colors::Lwhite;

	float rounding = 7;
}

struct Products
{
	vec4 fade, icon;
	float time;
};

bool Items::Product(const std::string& label, const std::string& expirationText, ProductStatus status, ID3D11ShaderResourceView* tex, const vec4& expirationColor)
{
	const auto& window = GetCurrentWindow();
	using namespace productcol;

	vec2 pos = window->DC.CursorPos;
	vec2 size = vec2(210, 125);

	bool r = InvisibleButton(label.c_str(), size);
	bool act = IsItemActive(); bool hov = IsItemHovered();

	vec4 statusCol = GetFromMap(ProductColors, status);
	std::string statusProd = GetFromMap(ProductLabels, status);

	window->DrawList->AddImageRounded((ImTextureID)tex, pos, pos + size, {}, { 1,1 }, h->CO(colors::White), rounding);

	const int idx = window->DrawList->VtxBuffer.Size;
	window->DrawList->AddRectFilled(pos, pos + size, h->CO(colors::White), rounding);
	const int idx1 = window->DrawList->VtxBuffer.Size;

	vec4 fadecol = hov ? fadeHov : fadeC;
	vec4 iconcol = hov ? iconHov : iconC;

	static std::map<std::string, Products> anim;
	auto& it = anim.emplace(label, Products(fadecol, iconcol)).first->second;

	it.time = gui->xtime(gui->GlobalSpeed);
	it.fade = ImLerp(it.fade, fadecol, it.time);
	it.icon = ImLerp(it.icon, iconcol, it.time);

	vec2 start(0, pos.y);
	vec2 end(0, size.y);
	ShadeVertsLinearColorGradientWithAlpha(window->DrawList, idx, idx1, start, start + end, h->CO(colors::Transparent), h->CO(it.fade));

	PushFont(fonts->InterS[0]);
	window->DrawList->AddText(pos + vec2(6, 6), h->CO(colors::Lwhite), label.c_str());
	PopFont();

	PushFont(fonts->InterM[1]);
	// Status first (higher row)
	window->DrawList->AddText(pos + vec2(6, size.y - h->CT("Status: ").y - 27), h->CO(colors::Lwhite), "Status: ");

	vec4 renderStatusCol = statusCol;
	if (status == ProductStatus::Updating)
	{
		float t = (sinf((float)ImGui::GetTime() * 6.2831853f) * 0.5f + 0.5f); // smooth pulse
		renderStatusCol.w = ImLerp(0.35f, 1.0f, t);
	}
	window->DrawList->AddText(pos + vec2(6 + h->CT("Status: ").x, size.y - h->CT("Status: ").y - 27), h->CO(renderStatusCol), statusProd.c_str());

	// Then time left / expiry
	window->DrawList->AddText(pos + vec2(6, size.y - h->CT("Time left: ").y - 10), h->CO(colors::Lwhite), "Time left: ");
	window->DrawList->AddText(pos + vec2(6 + h->CT("Time left: ").x, size.y - h->CT("Time left: ").y - 10), h->CO(expirationColor), expirationText.c_str());
	PopFont();

	if (hov)
		SetMouseCursor(ImGuiMouseCursor_Hand);

	float padding = 5;
	vec2 sqSize = vec2(40, 40);

	window->DrawList->AddRectFilled(pos + size - sqSize - vec2(padding, padding), pos + size - vec2(padding, padding ), h->CO(colors::ItemBg), 7);

	PushFont(fonts->Icons[3]);
	window->DrawList->AddText((pos + size - sqSize - vec2(padding, padding) + pos + size - vec2(padding, padding)) * 0.5f - h->CT(PLAY) * 0.5f, h->CO(it.icon), PLAY);
	PopFont();

	return r;
}