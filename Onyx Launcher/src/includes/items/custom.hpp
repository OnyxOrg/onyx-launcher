#include "../core/colors.hpp"

enum AnnouncementStatus
{
	updated,
	feature,
	bugfix,
	online,
	offline,
	updating
};

enum ProductStatus
{
	Online,
	Offline,
	Updating
};

inline std::map<AnnouncementStatus, vec4> AnnouncementColors =
{
	{ AnnouncementStatus::updated, colors::Main },
	{ AnnouncementStatus::feature, colors::Green},
	{ AnnouncementStatus::bugfix, colors::Blue },
	{ AnnouncementStatus::online, colors::Green },
	{ AnnouncementStatus::offline, colors::Red },
	{ AnnouncementStatus::updating, colors::Yellow }
}; // add more if needed

inline std::map<AnnouncementStatus, std::string> AnnouncementLabels =
{
	{ AnnouncementStatus::updated, "UPDATED" },
	{ AnnouncementStatus::feature, "FEATURE" },
	{ AnnouncementStatus::bugfix, "BUG FIX" }
};

inline std::map<ProductStatus, vec4> ProductColors =
{
	{ ProductStatus::Online, colors::Green },
	{ ProductStatus::Offline, colors::Red },
	{ ProductStatus::Updating, colors::Yellow }
};

inline std::map<ProductStatus, std::string> ProductLabels =
{
	{ ProductStatus::Online, "Online" },
	{ ProductStatus::Offline, "Offline" },
	{ ProductStatus::Updating, "Updating" }
};

//inline std::map

template<typename T, typename K, typename Z>
inline T GetFromMap(std::map<K, T> map, Z status)
{
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		if (it->first == status)
			return it->second;
	}

	OutputDebugStringA("Type error");
}

struct Custom
{
	void Begin(const std::string& name);
	void End();
	void BeginChild(const std::string& label, const vec2& size);
	void BeginChild2(const std::string& label, const vec2& size);
	void EndChild();
	void Banner(const std::string& title, const std::string& message, ID3D11ShaderResourceView* tex);

	DWORD WindowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollWithMouse;
}; inline std::unique_ptr<Custom> custom = std::make_unique<Custom>();

typedef ImDrawFlags Flags;
struct Draw
{
	void Text(const std::string& text, const vec4& col);
	void Rect(const vec2& pos, const vec2& size, const vec4& col, const float& rounding, Flags flags = 0, float border = 1);
	void Circle(const vec2& center, float rad, const vec4& col, Flags flags = 0, float border = 1);
	void Image(const vec2& pos, const vec2& size, const vec4& col, ID3D11ShaderResourceView* tex, const float& rounding);
}; inline std::unique_ptr<Draw> draw = std::make_unique<Draw>();

struct Items
{
	bool Input(const std::string& name, const std::string& iconA, const std::string& iconB, char* buf, size_t bufSize, ImGuiInputTextFlags flags = 0);
	bool Checkbox(const std::string& label, bool* v);
	bool Button(const std::string& label, const vec2& size);
	bool TextButton(const std::string& text, const std::string& id, vec4 col = {0, 0, 0, 0});
	bool ImageButton(const std::string& label, ID3D11ShaderResourceView* image, vec2 imgSize);
	bool Tab(const std::string& label, const std::string& icon, bool cond);
	void Announcement(const std::string& title, const std::string& description, const std::string& date, AnnouncementStatus status);
	bool Product(const std::string& label, const std::string& expirationText, ProductStatus status, ID3D11ShaderResourceView* tex, const vec4& expirationColor);
	bool Profile(const std::string& name, const std::string& role, ID3D11ShaderResourceView* tex);
	bool ButtonIcon(const std::string& label, const std::string& icon, const vec2& size);
	void SetInputError(const std::string& name, bool error);
	void ClearInputError(const std::string& name);

}; inline std::unique_ptr<Items> items = std::make_unique<Items>();