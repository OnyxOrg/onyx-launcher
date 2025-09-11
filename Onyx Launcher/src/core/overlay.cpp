#include "includes/core/overlay.hpp"

static bool g_overlayActive = false;

void RenderLoadingOverlay()
{
	RenderLoadingOverlay("Signing in...");
}

// Internal implementation with alpha control
static void RenderLoadingOverlayInternal(const char* label, float alphaMultiplier)
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

	// Fade-in support
	static double s_lastCallTime = -1.0;
	static double s_fadeStartTime = 0.0;
	const float kFadeInSeconds = 0.25f;
	double now = ImGui::GetTime();
	bool firstAfterGap = (s_lastCallTime < 0) || (now - s_lastCallTime > 0.5);
	if (firstAfterGap)
		s_fadeStartTime = now;
	s_lastCallTime = now;
	float fade = (float)std::min(1.0, (now - s_fadeStartTime) / (double)kFadeInSeconds);
	fade *= ImClamp(alphaMultiplier, 0.0f, 1.0f);

	// Dim background with fade
	dl->AddRectFilled(winPos, winPos + winSize, h->CO({ 0, 0, 0, 0.75f * fade }));

	// Spinner
	ImVec2 center = winPos + ImVec2(winSize.x * 0.5f, winSize.y * 0.5f - 10);
	float t = (float)ImGui::GetTime();
	float radius = 18.0f;
	float start = t * 4.0f;
	float end = start + IM_PI * 1.35f;
	dl->PathClear();
	dl->PathArcTo(center, radius, start, end, 48);
	vec4 stroke = { colors::Main.x, colors::Main.y, colors::Main.z, colors::Main.w * fade };
	dl->PathStroke(h->CO(stroke), 0, 3.0f);

	// Label
	const char* text = (label && label[0] != '\0') ? label : "Signing in...";
	vec2 ts = h->CT(text);
	ImVec2 tpos = center + ImVec2(-ts.x * 0.5f, radius + 12);
	vec4 txt = { colors::White.x, colors::White.y, colors::White.z, colors::White.w * fade };
	dl->AddText(tpos, h->CO(txt), text);

	// Block interactions behind the overlay by capturing mouse input
	SetCursorPos(ImVec2(0, 0));
	InvisibleButton("##overlay_blocker", winSize);

	End();

	// Mark overlay as active for this frame
	g_overlayActive = true;
}

void RenderLoadingOverlay(const char* label)
{
	RenderLoadingOverlayInternal(label, 1.0f);
}

void RenderLoadingOverlayEx(const char* label, float alphaMultiplier)
{
	RenderLoadingOverlayInternal(label, alphaMultiplier);
}

bool IsOverlayActive()
{
	return g_overlayActive;
}

void SetOverlayActive(bool active)
{
	g_overlayActive = active;
}

bool IsOverlayDragging()
{
	// Not implemented; keep for API compatibility if needed later
	return false;
}