//#define WINDOW_BG
#define IMGUI_DEFINE_MATH_OPERATORS
#define NOMINMAX

#define EYE_SLASHED "a"
#define EYE         "b"
#define CLOSE       "c"
#define MINUS       "d"
#define USER        "e"
#define BOOKMARK    "f"
#define HOME        "g"
#define KEY			"h"
#define PLAY		"i"
#define DISCORD		"j"

#pragma comment(lib, "D3DX11.lib")
#pragma comment(lib, "Shell32.lib")

#include <iostream>
#include <windows.h>
#include <shellapi.h>
#include <d3d11.h>
#include <D3DX11tex.h>
#include <dwmapi.h>
#include <map>
#include <algorithm>
#include <vector>
#include <utility>
#include <tuple>

#include "imgui.h" 
#include "imgui_internal.h"
#include "imgui_freetype.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "data/fonts.h"
#include "data/images.h"

using namespace ImGui;

typedef ImVec2 vec2;
typedef ImVec4 vec4;
typedef ImColor rgba;

#define _size(v) (sizeof(v) / sizeof(v[0]))

#define EFONT(PDATA, FSIZE, CFG) \
    (io.Fonts->AddFontFromMemoryTTF(PDATA, sizeof(PDATA), FSIZE, &CFG))

#define EIMAGE(DVC, PDATA, TARGET) \
 (D3DX11CreateShaderResourceViewFromMemory(DVC, PDATA, sizeof(PDATA), nullptr, nullptr, &TARGET, nullptr))

struct Gui
{
	vec2 Size = { 335, 485 };
	vec2 SizeOnChange = Size;

	float Rounding = 5;
	float GlobalSpeed = 10;
	float vtime(float speed) { return GetIO().DeltaTime * speed; }
	float xtime(float speed) { return speed / GetIO().Framerate; }

	void MoveWindowW();
	void SmoothResize(RECT rc);
	void Style();
}; inline std::unique_ptr<Gui> gui = std::make_unique<Gui>();


struct Window
{
public:
	HWND hWindow;
	BOOL Initialize(LPCWSTR WindowName);
	BOOL RenderLoop();
	BOOL CleanWindow();
	BOOL CleanImGui();

	UINT ResizeW;
	UINT ResizeH;
	BOOL IsRendering;
	ID3D11Device* Dvc;
	VOID Blur();

#ifndef WINDOW_BG
		vec2& size = gui->Size;
#else
	vec2 size = { 1200, 960 };
#endif

private:
	BOOL RegisterWindowClass();
	BOOL Create();
	BOOL CreateDeviceD3D();
	BOOL CreateRenderTarget();
	BOOL CleanDeviceD3D();
	BOOL CleanRenderTarget();
	BOOL InitializeImGui();
	BOOL InitializeWindow();

	BOOL HideBackground;
	WNDCLASSEXW WindowClass;

	vec4 ClearColor;
	LPCWSTR WindowName;

	ID3D11DeviceContext* Ctx;
	IDXGISwapChain* SwapChain;
	ID3D11RenderTargetView* TargetView;
	bool SwapChainOcculed;
}; inline std::unique_ptr<Window> window = std::make_unique<Window>();

struct Fonts
{
	std::vector<float> InterMSizes = { 11, 13, 14, 15, 18 };
	std::vector<float> InterSSizes = { 14, 18 };
	std::vector<float> IconSSizes = { 16, 20, 19, 24, 18 };

	std::vector<ImFont*> InterM = std::vector<ImFont*>(InterMSizes.size());
	std::vector<ImFont*> InterS = std::vector<ImFont*>(InterSSizes.size());
	std::vector<ImFont*> Icons = std::vector<ImFont*>(IconSSizes.size());

	ImFont* Pass;
	ImFont* Xirod;
	ImFont* RennerM;
	ImFont* RennerRole; // font used only for the user role text
	bool RenderFonts();
}; inline std::unique_ptr<Fonts> fonts = std::make_unique<Fonts>();

struct Images
{
	ID3D11ShaderResourceView* inputGlow;
	ID3D11ShaderResourceView* googleIcon;
	ID3D11ShaderResourceView* discordIcon;

	ID3D11ShaderResourceView* logo;
	ID3D11ShaderResourceView* banner;
	ID3D11ShaderResourceView* product;

	ID3D11ShaderResourceView* profilePic;
}; inline std::unique_ptr<Images> images = std::make_unique<Images>();

struct Helper
{
	ImVec2 CT(const std::string& TEXT)															{ return CalcTextSize(TEXT.c_str()); }
	ImU32 CO(const ImVec4& COL)																	{ return GetColorU32(COL); }
}; inline std::unique_ptr<Helper> h = std::make_unique<Helper>();

enum tabs
{
	login,
	registerr,
	home
};

enum subtabs
{
	none,
	dashboard,
	library,
	profile
};

class Alpha
{
public:
	int index, tab;
	float alpha;

	bool Render();
	int GetLastTab();
private:
	float time;
	int last;
};

