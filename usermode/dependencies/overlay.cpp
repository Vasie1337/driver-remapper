#include "overlay.h"
#include <cheat/settings.hpp>

ID3D11Device* Overlay::device = nullptr;
ID3D11DeviceContext* Overlay::device_context = nullptr;
IDXGISwapChain* Overlay::swap_chain = nullptr;
ID3D11RenderTargetView* Overlay::render_targetview = nullptr;

HWND Overlay::overlay = nullptr;
WNDCLASSEX Overlay::wc = { };

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SYSCOMMAND:
	{
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	}

	case WM_DESTROY:
	{
		Overlay::DestroyImGui();
		Overlay::DestroyDevice();
		Overlay::DestroyOverlay();
		PostQuitMessage(0);
		return 0;
	}

	case WM_CLOSE:
	{
		Overlay::DestroyImGui();
		Overlay::DestroyDevice();
		Overlay::DestroyOverlay();
		return 0;
	}
	}

	return DefWindowProc(window, msg, wParam, lParam);
}

bool Overlay::CreateDevice()
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

	sd.OutputWindow = overlay;

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };

	HRESULT result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0U,
		featureLevelArray,
		2,
		D3D11_SDK_VERSION,
		&sd,
		&swap_chain,
		&device,
		&featureLevel,
		&device_context);

	if (result == DXGI_ERROR_UNSUPPORTED) {
		result = D3D11CreateDeviceAndSwapChain(
			nullptr,
			D3D_DRIVER_TYPE_WARP,
			nullptr,
			0U,
			featureLevelArray,
			2,
			D3D11_SDK_VERSION,
			&sd,
			&swap_chain,
			&device,
			&featureLevel,
			&device_context);

		printf("[>>] DXGI_ERROR | Created with D3D_DRIVER_TYPE_WARP\n");
	}

	if (result != S_OK) {
		printf("Device Not Okay\n");
		return false;
	}

	ID3D11Texture2D* back_buffer{ nullptr };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

	if (back_buffer)
	{
		device->CreateRenderTargetView(back_buffer, nullptr, &render_targetview);
		back_buffer->Release();
		return true;
	}

	printf("[>>] Failed to create Device\n");
	return false;
}

void Overlay::DestroyDevice()
{
#define SAFE_RELEASE(x) if (x) { x->Release(); x = nullptr; }
	SAFE_RELEASE(device_context);
	SAFE_RELEASE(swap_chain);
}

void Overlay::CreateOverlay()
{
	wc.cbSize = sizeof(wc);
	wc.style = CS_CLASSDC;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = GetModuleHandleA(0);
	wc.lpszClassName = "VasieFiveM";

	RegisterClassEx(&wc);

	overlay = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
		wc.lpszClassName,
		"VasieFiveM",
		WS_POPUP,
		0,
		0,
		GetSystemMetrics(SM_CXSCREEN),
		GetSystemMetrics(SM_CYSCREEN),
		NULL,
		NULL,
		wc.hInstance,
		NULL
	);

	if (overlay == 0)
	{
		printf("Failed to create Overlay\n");
		return;
	}

	SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	{
		RECT client_area{};
		RECT window_area{};

		GetClientRect(overlay, &client_area);
		GetWindowRect(overlay, &window_area);

		POINT diff{};
		ClientToScreen(overlay, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(overlay, &margins);
	}

	ShowWindow(overlay, SW_SHOW);
	UpdateWindow(overlay);
}

void Overlay::DestroyOverlay()
{
	DestroyWindow(overlay);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
}

bool Overlay::CreateImGui()
{
	ImGui::CreateContext();
	ImGui::StyleColorsDark();

	if (!ImGui_ImplWin32_Init(overlay)) {
		printf("Failed ImGui_ImplWin32_Init\n");
		return false;
	}

	if (!ImGui_ImplDX11_Init(device, device_context)) {
		printf("Failed ImGui_ImplDX11_Init\n");
		return false;
	}

	ImGuiStyle& style = ImGui::GetStyle();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	style.WindowRounding = 4.f;
	style.ChildRounding = 4.f;
	style.FrameRounding = 3.f;
	style.PopupRounding = 3.f;
	style.GrabRounding = 3.f;
	style.TabRounding = 3.f;
	style.ScrollbarRounding = 1.f;

	style.ButtonTextAlign = { 0.5f, 0.5f };
	style.WindowTitleAlign = { 0.5f, 0.5f };
	style.FramePadding = { 6.0f, 6.0f };
	style.ItemSpacing = { 9.0f, 9.0f };
	style.WindowPadding = { 9.0f, 9.0f };
	style.ItemInnerSpacing = { 8.0f, 4.0f };

	style.WindowBorderSize = 1.f;
	style.FrameBorderSize = 2.f;

	style.ScrollbarSize = 12.f;
	style.GrabMinSize = 8.f;

	style.Colors[ImGuiCol_WindowBg] = ImAdd::HexToColorVec4(0x111111, 1.0f);
	style.Colors[ImGuiCol_ChildBg] = ImAdd::HexToColorVec4(0x151515, 1.0f);
	style.Colors[ImGuiCol_PopupBg] = ImAdd::HexToColorVec4(0x111111, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImAdd::HexToColorVec4(0x191919, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImAdd::HexToColorVec4(0x191919, 1.0f);
	style.Colors[ImGuiCol_TitleBg] = style.Colors[ImGuiCol_MenuBarBg];
	style.Colors[ImGuiCol_TitleBgActive] = style.Colors[ImGuiCol_MenuBarBg];

	style.Colors[ImGuiCol_Border] = ImAdd::HexToColorVec4(0x1F1F1F, 1.0f);
	style.Colors[ImGuiCol_Separator] = style.Colors[ImGuiCol_Border];

	style.Colors[ImGuiCol_CheckMark] = ImAdd::HexToColorVec4(0x151515, 1.0f);
	style.Colors[ImGuiCol_Text] = ImAdd::HexToColorVec4(0xFFFFFF, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImAdd::HexToColorVec4(0x616161, 1.0f);

	style.Colors[ImGuiCol_SliderGrab] = ImAdd::HexToColorVec4(0xFF8830, 1.0f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImAdd::HexToColorVec4(0xFF8830, 0.7f);

	style.Colors[ImGuiCol_Button] = ImAdd::HexToColorVec4(0x191919, 1.0f);
	style.Colors[ImGuiCol_ButtonHovered] = ImAdd::HexToColorVec4(0x191919, 0.7f);
	style.Colors[ImGuiCol_ButtonActive] = ImAdd::HexToColorVec4(0x191919, 0.5f);

	style.Colors[ImGuiCol_FrameBg] = style.Colors[ImGuiCol_Button];
	style.Colors[ImGuiCol_FrameBgHovered] = style.Colors[ImGuiCol_ButtonHovered];
	style.Colors[ImGuiCol_FrameBgActive] = style.Colors[ImGuiCol_ButtonActive];

	style.Colors[ImGuiCol_Header] = style.Colors[ImGuiCol_Button];
	style.Colors[ImGuiCol_HeaderHovered] = style.Colors[ImGuiCol_ButtonHovered];
	style.Colors[ImGuiCol_HeaderActive] = style.Colors[ImGuiCol_ButtonActive];

	ImFontConfig fa_config; fa_config.MergeMode = true; fa_config.PixelSnapH = true;
	ImFontConfig cfg;
	ImFont* mainFont = io.Fonts->AddFontFromMemoryCompressedTTF(Poppins_Medium_compressed_data, Poppins_Medium_compressed_size, 16, &cfg, io.Fonts->GetGlyphRangesDefault());

	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA, 0 };
	static const ImWchar icons_ranges_brands[] = { ICON_MIN_FAB, ICON_MAX_16_FAB, 0 };

	io.Fonts->AddFontFromMemoryCompressedTTF(fa6_solid_compressed_data, fa6_solid_compressed_size, 14, &fa_config, icons_ranges);
	io.Fonts->AddFontFromMemoryCompressedTTF(fa_brands_400_compressed_data, fa_brands_400_compressed_size, 14, &fa_config, icons_ranges_brands);

	return true;
}

void Overlay::DestroyImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Overlay::StartRender()
{
	MSG msg;
	while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	Focussed = IsWindowInForeground(game_window) || IsWindowInForeground(overlay);

	if (GetAsyncKeyState(VK_END) & 1)
		ExitProcess(1);

	if (GetAsyncKeyState(VK_INSERT) & 1)
		ShowMenu = !ShowMenu;

	LONG NewLong = WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_LAYERED;

	if (ShowMenu && Focussed)
		NewLong = WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT;

	SetWindowLong(overlay, GWL_EXSTYLE, NewLong);
}

void Overlay::EndRender()
{
	ImGui::Render();

	float color[4]{ 0, 0, 0, 0 };

	device_context->OMSetRenderTargets(1, &render_targetview, nullptr);
	device_context->ClearRenderTargetView(render_targetview, color);

	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	swap_chain->Present(1U, 0U);
}

void Overlay::Render()
{
    ImGuiStyle& style = ImGui::GetStyle();

    static float SideBarWidth = 160;
    float FooterHeight = ImGui::GetFrameHeight();
    float HeaderHeight = ImGui::GetFrameHeight() + style.WindowPadding.y * 2;

    static ImVec2 MenuSize = ImVec2(500, 500);

    ImGui::SetNextWindowSize(MenuSize, ImGuiCond_Once);
    ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize / 2 - MenuSize / 2, ImGuiCond_Once);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("menu", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    ImGui::PopStyleVar(2);

    {
        ImVec2 pos = ImGui::GetWindowPos();
        ImVec2 size = ImGui::GetWindowSize();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(pos, pos + ImVec2(SideBarWidth, size.y), ImGui::GetColorU32(ImGuiCol_ChildBg), style.WindowRounding, ImDrawFlags_RoundCornersLeft);
        drawList->AddRectFilled(pos + ImVec2(SideBarWidth, 0), pos + ImVec2(size.x, HeaderHeight), ImGui::GetColorU32(ImGuiCol_ChildBg), style.WindowRounding, ImDrawFlags_RoundCornersTopRight);
        drawList->AddRectFilled(pos + ImVec2(SideBarWidth, HeaderHeight), pos + ImVec2(size.x, size.y - FooterHeight), ImGui::GetColorU32(ImGuiCol_WindowBg), style.WindowRounding, ImDrawFlags_RoundCornersNone);
        drawList->AddRectFilled(pos + ImVec2(SideBarWidth, size.y - FooterHeight), pos + size, ImGui::GetColorU32(ImGuiCol_ChildBg), style.WindowRounding, ImDrawFlags_RoundCornersBottomRight);

        if (style.WindowBorderSize > 0) {
            drawList->AddLine(pos + ImVec2(SideBarWidth - style.WindowBorderSize, style.WindowBorderSize), pos + ImVec2(SideBarWidth - style.WindowBorderSize, size.y - style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
            drawList->AddLine(pos + ImVec2(SideBarWidth, HeaderHeight - style.WindowBorderSize), pos + ImVec2(size.x - style.WindowBorderSize, HeaderHeight - style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
            drawList->AddLine(pos + ImVec2(SideBarWidth, size.y - FooterHeight + style.WindowBorderSize), pos + ImVec2(size.x - style.WindowBorderSize, size.y - FooterHeight + style.WindowBorderSize), ImGui::GetColorU32(ImGuiCol_Border), style.WindowBorderSize);
            drawList->AddRect(pos, pos + size, ImGui::GetColorU32(ImGuiCol_Border), style.WindowRounding);
        }

        drawList->AddText(pos + ImVec2(SideBarWidth + style.FramePadding.x, size.y - FooterHeight + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_TextDisabled), "VasieFiveM");
        drawList->AddText(pos + ImVec2(size.x - ImGui::CalcTextSize("v2.0").x - style.FramePadding.x, size.y - FooterHeight + style.FramePadding.y), ImGui::GetColorU32(ImGuiCol_SliderGrab), "v2.0");
    }

    {
        ImGui::BeginChild("SideBar", ImVec2(SideBarWidth, 0), ImGuiChildFlags_Border, ImGuiWindowFlags_NoBackground);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.ChildRounding);
            ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_Header]);
            {
                ImAdd::SeparatorText("Aim");
                ImAdd::RadioButtonIcon("aimbot", ICON_FA_GUN, "Aimbot", &CurrentPage, MenuPage_Aimbot, ImVec2(-0.1f, 0));

                ImAdd::SeparatorText("Visuals");
                ImAdd::RadioButtonIcon("esp", ICON_FA_USERS_VIEWFINDER, "Esp", &CurrentPage, MenuPage_Esp, ImVec2(-0.1f, 0));
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
        ImGui::SameLine(SideBarWidth);
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("Header", ImVec2(0, HeaderHeight), ImGuiChildFlags_Border, ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar);
            {

            }
            ImGui::EndChild();
            ImGui::SetCursorPosY(HeaderHeight);
            ImGui::BeginChild("Main", ImVec2(0, ImGui::GetWindowHeight() - FooterHeight - HeaderHeight), ImGuiChildFlags_Border, ImGuiWindowFlags_NoBackground);
            {
                float fGroupWidth = (ImGui::GetWindowWidth() - style.ItemSpacing.x - style.WindowPadding.x * 2) / 2;
                float fGroupHeight = (ImGui::GetWindowHeight() - style.ItemSpacing.y - style.WindowPadding.y * 2) / 2;

                if (CurrentPage == MenuPage_Aimbot)
                {
                    ImGui::BeginChild("group1", ImVec2(0, 0), ImGuiChildFlags_Border);
                    {
                        ImGui::TextDisabled("Aimbot");
                        {
							ImAdd::Checkbox("Enable", &EnableAimbot);
							ImAdd::SliderFloat("Smooth", &Smooth, 1.f, 15.f);
							ImAdd::SliderFloat("FOV", &FOV, 30.f, 200.f);
							ImAdd::Combo("Bone", &AimBone, AimBonesList, IM_ARRAYSIZE(AimBonesList));
							ImAdd::KeyBind("Aim key", &aimKey);
                        }
                    }
                    ImGui::EndChild();
                }
                else if (CurrentPage == MenuPage_Esp)
                {
                    ImGui::BeginChild("group1", ImVec2(0, 0), ImGuiChildFlags_Border);
                    {
                        ImGui::TextDisabled("Esp");
						{
							ImAdd::Checkbox("Enable", &EnableEsp); 
							ImAdd::Checkbox("Only Players", &OnlyPlayer);
							ImAdd::Checkbox("HealthBar", &HealthBar);
							ImAdd::Checkbox("WeaponName", &WeaponName);

							ImAdd::Checkbox("Skeleton", &DrawSkeleton);
							if (DrawSkeleton) {
								ImGui::SameLine();
								ImAdd::ColorEdit4("SkelColor", reinterpret_cast<float*>(&SkelColor));
							}
							ImAdd::Checkbox("Box", &DrawBox);
							if (DrawBox) {
								ImGui::SameLine();
								ImAdd::ColorEdit4("BoxColor", reinterpret_cast<float*>(&BoxColor));
								ImAdd::Checkbox("Filled", &Filled);
							}
						}
                    }
                    ImGui::EndChild();
                }
                else
                {
                    ImGui::Text("404");
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndGroup();
    }
    ImGui::End();
}

void Overlay::SetForeground(HWND window)
{
	if (!IsWindowInForeground(window))
		BringToForeground(window);
}