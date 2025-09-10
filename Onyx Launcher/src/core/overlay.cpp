#include "includes/core/overlay.hpp"

void RenderLoadingOverlay()
{
	const auto& wnd = GetCurrentWindow();
	ImDrawList* dl = wnd->DrawList;
	ImVec2 pos = wnd->Pos;
	ImVec2 size = wnd->Size;

	// Dim background
	dl->AddRectFilled(pos, pos + size, h->CO({ 0, 0, 0, 0.75f }));

	// Spinner
	ImVec2 center = pos + ImVec2(size.x * 0.5f, size.y * 0.5f - 10);
	float t = (float)ImGui::GetTime();
	float radius = 18.0f;
	float start = t * 4.0f;
	float end = start + IM_PI * 1.35f;
	dl->PathClear();
	dl->PathArcTo(center, radius, start, end, 48);
	dl->PathStroke(h->CO(colors::Main), 0, 3.0f);

	// Label
	vec2 ts = h->CT("Signing in...");
	ImVec2 tpos = center + ImVec2(-ts.x * 0.5f, radius + 12);
	dl->AddText(tpos, h->CO(colors::White), "Signing in...");
}


