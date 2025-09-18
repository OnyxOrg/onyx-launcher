#include "includes/core/items/custom.hpp"
#include <sstream>

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
	vec2 baseSize = { window->Size.x - GetCursorPosX() * 2, 135 }; // Base height

	// Calculate text height for proper wrapping
	PushFont(fonts->announcementDescriptionFont);
	float availableWidth = baseSize.x - 20; // 10px margin on each side
	PushTextWrapPos(GetCursorPosX() + availableWidth);
	vec2 textSize = CalcTextSize(description.c_str(), nullptr, false, availableWidth);
	PopTextWrapPos();
	PopFont();

	// Calculate dynamic height based on text content
	float textHeight = textSize.y + 20; // Add some padding
	float minHeight = 135.0f;
	float dynamicHeight = std::max(minHeight, textHeight + 60); // 60px for title and padding
	
	vec2 size = { baseSize.x, dynamicHeight };

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
	// Description uses independent font with manual text wrapping
	PushFont(fonts->announcementDescriptionFont);
	
	// Manual text wrapping implementation
	std::string wrappedText = description;
	vec2 textPos = pos + vec2(10, 45);
	float lineHeight = fonts->announcementDescriptionFont->FontSize + 2; // Add small line spacing
	float maxWidth = availableWidth;
	
	// Split text into lines that fit within the available width
	std::vector<std::string> lines;
	std::string currentLine = "";
	std::istringstream iss(description);
	std::string word;
	
	while (iss >> word) {
		std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
		vec2 testSize = h->CT(testLine);
		
		if (testSize.x <= maxWidth) {
			currentLine = testLine;
		} else {
			if (!currentLine.empty()) {
				lines.push_back(currentLine);
				currentLine = word;
			} else {
				// Word is too long, force it on its own line
				lines.push_back(word);
			}
		}
	}
	if (!currentLine.empty()) {
		lines.push_back(currentLine);
	}
	
	// Draw each line
	for (size_t i = 0; i < lines.size(); i++) {
		vec2 linePos = textPos + vec2(0, i * lineHeight);
		window->DrawList->AddText(linePos, h->CO(descriptionC), lines[i].c_str());
	}
	
	PopFont();

}


