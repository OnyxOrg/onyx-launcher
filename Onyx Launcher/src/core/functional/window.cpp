#include "../../includes/core/main.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;

		window->ResizeW = (UINT)LOWORD(lParam);
		window->ResizeH = (UINT)HIWORD(lParam);

		return 0;
	
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

BOOL Window::RegisterWindowClass()
{
	this->WindowClass = {
		sizeof(this->WindowClass),
		CS_CLASSDC,
		WndProc,
		0L, 0L,
		GetModuleHandle(nullptr),
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		this->WindowName,
		nullptr
	};

	if (!::RegisterClassExW(&this->WindowClass))
		return false;

	return true;
}

BOOL Window::Create()
{
	if (!this->RegisterWindowClass())
	{
		printf("Error registering the window class\n");
		return false;
	}

	DWORD Style = (this->HideBackground == true) ? WS_POPUP : WS_OVERLAPPEDWINDOW;
	this->hWindow = CreateWindowW(
		this->WindowClass.lpszClassName,
		this->WindowName,
		Style,
		(GetSystemMetrics(SM_CXSCREEN) / 2) - this->size.x / 2,
		(GetSystemMetrics(SM_CYSCREEN) / 2) - this->size.y / 2,
		(this->HideBackground == true) ? 0 : this->size.x,
		(this->HideBackground == true) ? 0 : this->size.y,
		nullptr,
		nullptr,
		this->WindowClass.hInstance,
		nullptr
	);

	if (!this->hWindow)
		return false;

	if (this->HideBackground)
	{
		SetWindowLongA(this->hWindow, GWL_EXSTYLE, GetWindowLong(this->hWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLayeredWindowAttributes(this->hWindow, RGB(0, 0, 0), 255, LWA_ALPHA);

		MARGINS margins = { -1 };
		DwmExtendFrameIntoClientArea(this->hWindow, &margins);
	}

	this->ClearColor = (this->HideBackground == true) ? 
		vec4(0.1f, 0.1f, 0.1f, 0.1) : 
		vec4(0.086f, 0.094f, 0.125f, 1.0f);

	return true;
}

BOOL Window::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;

	if (FAILED(this->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
		return false;

	if (FAILED(this->Dvc->CreateRenderTargetView(pBackBuffer, nullptr, &this->TargetView)))
	{
		pBackBuffer->Release();
		return false;
	}

	pBackBuffer->Release();

	return true;
}

BOOL Window::CreateDeviceD3D()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = this->hWindow;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &this->SwapChain, &this->Dvc, &featureLevel, &this->Ctx);
	if (res == DXGI_ERROR_UNSUPPORTED)
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &this->SwapChain, &this->Dvc, &featureLevel, &this->Ctx);
	if (res != S_OK)
		return false;

	this->CreateRenderTarget();
	return true;
}

BOOL Window::CleanRenderTarget()
{
	if (this->TargetView) 
	{
		this->TargetView->Release(); 
		this->TargetView = nullptr;
		return true;
	}

	return false;
}

BOOL Window::CleanDeviceD3D()
{
	this->CleanRenderTarget();
	if (this->SwapChain) { this->SwapChain->Release(); this->SwapChain = nullptr; }
	if (this->Ctx) { this->Ctx->Release(); this->Ctx = nullptr; }
	if (this->Dvc) { this->Dvc->Release(); this->Dvc = nullptr; }

	return true;
}

BOOL Window::InitializeWindow()
{
	if (!this->Create())
	{
		return 1;
	}

	if (!this->CreateDeviceD3D())
	{
		this->CleanDeviceD3D();
		::UnregisterClassW(this->WindowClass.lpszClassName, this->WindowClass.hInstance);
		return 1;
	}

	::ShowWindow(this->hWindow, SW_SHOWDEFAULT);
	::UpdateWindow(this->hWindow);

	return true;
}

BOOL Window::InitializeImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(this->hWindow))
		return false;
	

	if (!ImGui_ImplDX11_Init(this->Dvc, this->Ctx))
		return false;

	auto& io = GetIO();
	io.IniFilename = NULL;

	return true;
}

BOOL Window::RenderLoop()
{
	MSG msg;
	while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
	{
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
		if (msg.message == WM_QUIT)
			this->IsRendering = false;
	}

	if (this->ResizeW != 0 && this->ResizeH != 0)
	{
		this->CleanRenderTarget();
		this->SwapChain->ResizeBuffers(0, this->ResizeW, this->ResizeH, DXGI_FORMAT_UNKNOWN, 0);
		this->ResizeW = this->ResizeH = 0;
		this->CreateRenderTarget();
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();

	if (!this->IsRendering)
		return false;
	 
	return true;
}

BOOL Window::CleanImGui()
{
	ImGui::Render();
	const float clear_color_with_alpha[4] = { this->ClearColor.x * this->ClearColor.w, this->ClearColor.y * this->ClearColor.w, this->ClearColor.z * this->ClearColor.w, this->ClearColor.w };
	this->Ctx->OMSetRenderTargets(1, &this->TargetView, nullptr);
	this->Ctx->ClearRenderTargetView(this->TargetView, clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	HRESULT hr = this->SwapChain->Present(1, 0);
	this->SwapChainOcculed = (hr == DXGI_STATUS_OCCLUDED);

	return true;
}

BOOL Window::CleanWindow()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	this->CleanDeviceD3D();
	::DestroyWindow(this->hWindow);
	::UnregisterClassW(this->WindowClass.lpszClassName, this->WindowClass.hInstance);

	return true;
}

BOOL Window::Initialize(LPCWSTR WindowName)
{
#ifndef WINDOW_BG
	this->HideBackground = true;
#endif
	this->WindowName = WindowName;
	this->IsRendering = true;

	this->InitializeWindow();
	this->InitializeImGui();
	return true;
}