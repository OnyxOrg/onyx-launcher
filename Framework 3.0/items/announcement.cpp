#include "custom.hpp"

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

	vec4 statusCol = GetFromMap(AnnouncementColors, status);
	std::string infoLabel = GetFromMap(AnnouncementLabels, status);

	ItemSize(size);
	if (!ItemAdd({ pos, pos + size }, 0))
		return;
	ItemHoverable({ pos, pos + size }, 0, 0);

	window->DrawList->AddRectFilled(pos, pos + size, h->CO(bg), rounding);
	window->DrawList->AddRect(pos, pos + size, h->CO(border), rounding);

	PushFont(fonts->InterS[0]);

	vec2 titleSize = h->CT(title);
	window->DrawList->AddText(pos + vec2(10, 10), h->CO(titleC), title.c_str());

	PopFont();

	PushFont(fonts->InterM[2]);

	vec2 dateSize = h->CT(date);
	vec2 infoLabelSize = h->CT(infoLabel);

	window->DrawList->AddText(pos + vec2(size.x - dateSize.x - 10, 10 + (titleSize.y - dateSize.y) / 2), h->CO(descriptionC), date.c_str());
	window->DrawList->AddText(pos + vec2(10, 45), h->CO(descriptionC), description.c_str());

	window->DrawList->AddRectFilled(pos + vec2(10 + titleSize.x + 15, 10 - (21 - GetFontSize()) / 2), pos + vec2(10 + titleSize.x + 15 + infoLabelSize.x + 20, 10 - (21 - GetFontSize()) / 2 + 21), h->CO(statusCol), 1000);
	window->DrawList->AddText(pos + vec2(10 + titleSize.x + 15 + 10, 10 - (21 - GetFontSize()) / 2 + (21 - GetFontSize()) / 2), h->CO(colors::White), infoLabel.c_str());

	PopFont();
}