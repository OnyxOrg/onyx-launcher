#include "includes/core/items/custom.hpp"

void Custom::Begin(const std::string& ID)
{
#ifndef WINDOW_BG
	SetNextWindowPos({ 0, 0 }, ImGuiCond_Appearing);
#endif
	SetNextWindowSize((gui->Size));
	ImGui::Begin(ID.c_str(), NULL, WindowFlags);

	ImGuiWindow* windowW = GetCurrentWindow();
	windowW->DrawList->AddRectFilled(windowW->Pos, windowW->Pos + windowW->Size, h->CO(colors::Bg), gui->Rounding);

	gui->Style();
#ifndef WINDOW_BG
	gui->MoveWindowW();
#else
	gui->Size = ImLerp(gui->Size, gui->SizeOnChange, gui->vtime(10));
#endif
}

void Custom::Banner(const std::string& title, const std::string& message, ID3D11ShaderResourceView* tex)
{
	const auto& window = GetCurrentWindow();

	vec2 pos = window->DC.CursorPos;
	vec2 size = vec2(window->Size.x + 160 - GetCursorPosX() * 2, 180);

	ItemSize(size);
	ItemAdd({ pos, pos + size }, 0);
	ItemHoverable({ pos, pos + size }, 0, 0);
	
	window->DrawList->AddImageRounded((ImTextureID)tex, pos, pos + size, {}, { 1, 1 }, h->CO(colors::White), 15, 0);

	vec4 layer = rgba(0, 0, 0, 255 * 0.53);
	window->DrawList->AddRectFilled(pos, pos + size, h->CO(layer), 15);

	PushFont(fonts->InterS[1]);
	window->DrawList->AddText(pos + vec2(15, 16), h->CO(colors::White), title.c_str());
	PopFont();

	PushFont(fonts->RennerM);
	window->DrawList->AddText(pos + vec2(15, 48), h->CO(colors::Lwhite), message.c_str());
	PopFont();
}

void Custom::BeginChild(const std::string& label, const vec2& size)
{
	const auto& window = GetCurrentWindow();

	vec2 pos = window->DC.CursorPos;

	PushFont(fonts->InterS[1]);
	window->DrawList->AddText(pos, h->CO(colors::White), label.c_str());
	PopFont();

	SetNextWindowPos(pos + vec2(0, 32));
	ImGui::BeginChild(label.c_str(), size, 0, WindowFlags & ~(ImGuiWindowFlags_NoScrollWithMouse));
}

void Custom::BeginChild2(const std::string& label, const vec2& size)
{
	const auto& window = GetCurrentWindow();

	vec2 pos = window->DC.CursorPos;

	SetNextWindowPos(pos);
	ImGui::BeginChild(label.c_str(), size, 0, WindowFlags & ~(ImGuiWindowFlags_NoScrollWithMouse));

	window->DrawList->AddRectFilled(pos, pos + size, h->CO(colors::ItemBg), 15);
	window->DrawList->AddRect(pos, pos + size, h->CO(colors::Gray), 15);
}

void Custom::EndChild()
{
	ImGui::EndChild();
}

void Custom::End()
{
	ImGui::End();
}

void Draw::Text(const std::string& ID, const vec4& col)
{
	ImGuiWindow* window = GetCurrentWindow();

	vec2 pos = window->DC.CursorPos;
	vec2 size = h->CT(ID);

	ItemSize(size);
	ItemAdd({ pos, pos + size }, 0);

	window->DrawList->AddText(pos, h->CO(col), ID.c_str());
}

void Draw::Rect(const vec2& p1, const vec2& p2, const vec4& col, const float& rounding, Flags flags, float border)
{
	ImGuiWindow* window = GetCurrentWindow();

	SetCursorPos(p1);
	vec2 pos = window->DC.CursorPos;

	if (flags & ImDrawFlags_NoFill)
		window->DrawList->AddRect(pos, pos + p2, h->CO(col), rounding, flags, border);
	else
		window->DrawList->AddRectFilled(pos, pos + p2, h->CO(col), rounding, flags);
}

void Draw::Circle(const vec2& center, float rad, const vec4& col, Flags flags, float border)
{
	ImGuiWindow* window = GetCurrentWindow();

	SetCursorPos(center);
	vec2 pos = window->DC.CursorPos;

	if (flags & ImDrawFlags_NoFill)
		window->DrawList->AddCircle(center, rad, h->CO(col), 1000, border);
	else
		window->DrawList->AddCircleFilled(center, rad, h->CO(col), 1000);
}

void Draw::Image(const vec2& p1, const vec2& size, const vec4& col, ID3D11ShaderResourceView* tex, const float& rounding)
{
	ImGuiWindow* window = GetCurrentWindow();

	SetCursorPos(p1);
	vec2 pos = window->DC.CursorPos;

	window->DrawList->AddImageRounded((ImTextureID)tex, pos, pos + size, {}, { 1, 1 }, h->CO(col), rounding, 0);
}


