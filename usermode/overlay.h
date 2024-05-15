#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <ctime>

#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

#include <dependencies/Dear ImGui/imgui.h>
#include <dependencies/Dear ImGui/backends/imgui_impl_dx11.h>
#include <dependencies/Dear ImGui/backends/imgui_impl_win32.h>

class Overlay
{
public:
	static bool CreateDevice();
	static void DestroyDevice();

	static void CreateOverlay();
	static void DestroyOverlay();

	static bool CreateImGui();
	static void DestroyImGui();

	static void StartRender();
	static void EndRender();

	static void Render();

	inline static HWND game_window;
	static HWND overlay;

	static WNDCLASSEX wc;

	static bool IsWindowInForeground(HWND window) { return GetForegroundWindow() == window; }
	static bool BringToForeground(HWND window) { return SetForegroundWindow(window); }

	static void SetForeground(HWND window);

	static ID3D11Device* device;
	static ID3D11DeviceContext* device_context;
	static IDXGISwapChain* swap_chain;
	static ID3D11RenderTargetView* render_targetview;

	inline static bool RenderMenu;

	inline static bool shouldRun;

	inline static bool Focussed;
};