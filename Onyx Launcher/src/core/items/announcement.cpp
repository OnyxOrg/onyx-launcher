#include "includes/core/items/custom.hpp"

namespace announcementcol
{
	vec4 bg = colors::ItemBg;
	vec4 border = colors::Gray;

	vec4 titleC = colors::Lwhite;
	vec4 descriptionC = colors::Lwhite2;

	float rounding = 15;
}

void Items::Announcement(const std::string& title, const std::string& description, const std::string& date, AnnouncementStatus status)
{
	const auto& window = GetCurrentWindow();
	using namespace announcementcol;

	vec2 pos = window->DC.CursorPos;
	vec2 size = { window->Size.x - GetCursorPosX() * 2, 135 }; // if its not used in child change X size to 580 :<

	// Labels removed: no status badge rendering

	ItemSize(size);
	if (!ItemAdd({ pos, pos + size }, 0))
		return;
	ItemHoverable({ pos, pos + size }, 0, 0);

	window->DrawList->AddRectFilled(pos, pos + size, h->CO(bg), rounding);
	window->DrawList->AddRect(pos, pos + size, h->CO(border), rounding);

    // No status bar on the left; clean card

	// Title font remains the same
	PushFont(fonts->InterS[0]);

	vec2 titleSize = h->CT(title);
	window->DrawList->AddText(pos + vec2(10, 10), h->CO(titleC), title.c_str());

	PopFont();

	// Use configurable announcementStatusFont for date element
	PushFont(fonts->announcementStatusFont);

	vec2 dateSize = h->CT(date);

	window->DrawList->AddText(pos + vec2(size.x - dateSize.x - 10, 10 + (titleSize.y - dateSize.y) / 2), h->CO(descriptionC), date.c_str());
	PopFont();
	// Description uses independent font
	PushFont(fonts->announcementDescriptionFont);
	window->DrawList->AddText(pos + vec2(10, 45), h->CO(descriptionC), description.c_str());
	PopFont();

}


