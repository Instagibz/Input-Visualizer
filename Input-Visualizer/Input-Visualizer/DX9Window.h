#pragma once
#include <math.h>
#include <vector>
#include <string>
#include <chrono>

#include <d3d9.h>
#pragma comment(lib, "d3d9.lib")

#include <d3dx9.h>
#pragma comment(lib, "d3dx9.lib")

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

class DX9_Window {

public:
	LPDIRECT3D9			D3DInterface;
	LPDIRECT3DDEVICE9	D3DDevice;
	D3DDISPLAYMODE		D3DDisplayMode;

	LPD3DXLINE			D3DXLine;
	LPD3DXFONT			D3DFont;

	HWND				Window;
	WNDCLASSEX			WindowClass;

	struct FPS {
		unsigned long long FramesRendered, CurrentTime, LastTime, ElapsedTime;
		std::string CurrentFPS;
		double FrameTime;
		std::chrono::steady_clock::time_point BeginTime;
		std::chrono::steady_clock::time_point EndTime;
	} FPS;


	DX9_Window::DX9_Window(HINSTANCE Instance, int CmdShow, std::vector<int> Pos, std::vector<int> Size);
	DX9_Window::~DX9_Window(void);

	void DX9_Window::BeginScene(void);
	void DX9_Window::EndScene(void);

	void DX9_Window::FillRect(std::vector<int> Position, std::vector<int> Size, D3DCOLOR Color);
	void DX9_Window::DrawString(const std::vector<int> &Pos, const std::vector<int> &Size, const std::string &Text, const D3DCOLOR &Color);
	void DX9_Window::DisplayFPSCounter(std::vector<int> Pos, D3DCOLOR Color);
	void DX9_Window::DrawKeyboard(const std::vector<int> &Pos, std::vector<bool> &KBStruct);
	void DX9_Window::DrawBox(const std::vector<int> &Pos, const std::vector<int> &Size, const D3DCOLOR &Color, bool Centered);
	long double DX9_Window::MapRange(long double OldValue, long double OldMin, long double OldMax, long double NewMin, long double NewMax);
};