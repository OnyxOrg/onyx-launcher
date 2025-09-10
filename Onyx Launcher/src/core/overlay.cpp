#include "includes/core/overlay.hpp"

void RenderLoadingOverlay()
{
	// Anchor overlay to the current window area
	const auto& parent = GetCurrentWindow();
	ImVec2 pos = parent->Pos;
	ImVec2 size = parent->Size;

	// Create a dedicated overlay window to reliably capture inputs
	SetNextWindowPos(pos);
	SetNextWindowSize(size);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoNav;
	Begin("##loading_overlay", nullptr, flags);

	ImVec2 winPos = GetWindowPos();
	ImVec2 winSize = GetWindowSize();
	ImDrawList* dl = GetWindowDrawList();

	// Dim background
	dl->AddRectFilled(winPos, winPos + winSize, h->CO({ 0, 0, 0, 0.75f }));

	// Spinner
	ImVec2 center = winPos + ImVec2(winSize.x * 0.5f, winSize.y * 0.5f - 10);
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

	// Block interactions behind the overlay by capturing mouse input
	SetCursorPos(ImVec2(0, 0));
	InvisibleButton("##overlay_blocker", winSize);

	End();
}