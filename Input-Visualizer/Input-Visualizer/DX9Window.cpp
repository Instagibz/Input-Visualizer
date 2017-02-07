#include "DX9Window.h"

//Window Procedure
LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {

	switch (Message) {

	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}

					 break;
	}

	return DefWindowProc(Window, Message, WParam, LParam);
}

DX9_Window::DX9_Window(HINSTANCE Instance, int CmdShow, std::vector<int> Pos, std::vector<int> Size) {

	ZeroMemory(&this->WindowClass, sizeof(this->WindowClass));

	this->WindowClass.cbSize = sizeof(WNDCLASSEX);
	this->WindowClass.style = CS_HREDRAW | CS_VREDRAW;
	this->WindowClass.lpfnWndProc = WindowProc;
	this->WindowClass.hInstance = Instance;
	this->WindowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	this->WindowClass.hbrBackground = (HBRUSH)COLOR_WINDOW;
	this->WindowClass.lpszClassName = L"Utilitys";

	RegisterClassEx(&this->WindowClass);

	this->Window = CreateWindowEx(
		WS_EX_COMPOSITED,
		L"Utilitys",
		L"Input-Grabber",
		WS_SYSMENU, // WS_CAPTION | WS_SYSMENU
		Pos[0], Pos[1],
		Size[0], Size[1] + GetSystemMetrics(SM_CYCAPTION),
		NULL,
		NULL,
		Instance,
		NULL
	);

	MARGINS DWM_Margins;
	DWM_Margins.cxLeftWidth = -1;
	DWM_Margins.cxRightWidth = -1;
	DWM_Margins.cyBottomHeight = -1;
	DWM_Margins.cyTopHeight = -1;

	DwmExtendFrameIntoClientArea(Window, &DWM_Margins);

	ShowWindow(Window, CmdShow);

	this->D3DInterface = Direct3DCreate9(D3D_SDK_VERSION);

	D3DInterface->EnumAdapterModes(NULL, D3DFMT_A8R8G8B8, NULL, &this->D3DDisplayMode);

	D3DPRESENT_PARAMETERS D3DParameters = { NULL };

	D3DParameters.Windowed = true;
	D3DParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	D3DParameters.hDeviceWindow = this->Window;
	D3DParameters.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
	D3DParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	D3DParameters.MultiSampleQuality = NULL;
	D3DParameters.BackBufferFormat = D3DFMT_A8R8G8B8;
	D3DParameters.BackBufferWidth = Size[0];
	D3DParameters.BackBufferHeight = Size[1];

	this->D3DInterface->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		this->Window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&D3DParameters,
		&this->D3DDevice
	);

	D3DXCreateLine(this->D3DDevice, &this->D3DXLine);
	this->D3DXLine->SetWidth(2);
	this->D3DXLine->SetPattern(0xFFFFFFFF);
	this->D3DXLine->SetAntialias(false);

	D3DXCreateFont(
		this->D3DDevice,
		30,
		18,
		FW_NORMAL,
		1,
		false,
		DEFAULT_CHARSET,
		OUT_DEFAULT_PRECIS,
		ANTIALIASED_QUALITY, // ANTIALIASED_QUALITY
		DEFAULT_PITCH | FF_DONTCARE,
		L"Times New Roman",
		&this->D3DFont);

}


DX9_Window::~DX9_Window(void) {
	UnregisterClass(L"DX9 Window", NULL);
	DestroyWindow(this->Window);
	this->D3DDevice->Release();
	this->D3DInterface->Release();
}

void DX9_Window::BeginScene(void) {
	this->FPS.BeginTime = std::chrono::steady_clock::now();

	this->D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, NULL);
	this->D3DDevice->BeginScene();
}

void DX9_Window::EndScene(void) {
	this->D3DDevice->EndScene();
	this->D3DDevice->Present(NULL, NULL, NULL, NULL);
	this->FPS.EndTime = std::chrono::steady_clock::now();
	this->FPS.FrameTime = std::chrono::duration_cast<std::chrono::microseconds>(this->FPS.EndTime - this->FPS.BeginTime).count();
}

void DX9_Window::FillRect(std::vector<int> Position, std::vector<int> Size, D3DCOLOR Color) {
	D3DRECT Rect = { Position[0], Position[1], (Position[0] + Size[0]), (Position[1] + Size[1]) };

	this->D3DDevice->Clear(1, &Rect, D3DCLEAR_TARGET, Color, 0.0f, 0);
}

long double DX9_Window::MapRange(long double OldValue, long double OldMin, long double OldMax, long double NewMin, long double NewMax) {

	long double NewValue = 0.0, OldRange = 0.0, NewRange = 0.0;

	if (OldMin < 0) {
		if (OldMax >= 0) OldRange = (OldMin*-1) + OldMax;
		if (OldMax < 0) OldRange = (OldMin*-1) - (OldMax*-1);
	}
	if (OldMin >= 0) {
		if (OldMax >= 0) OldRange = OldMax - OldMin;
		if (OldMax < 0) OldRange = OldMin + (OldMax*-1);
	}

	if (NewMin < 0) {
		if (NewMax >= 0) NewRange = NewMax + (NewMin*-1);
		if (NewMax < 0) NewRange = (NewMin*-1) - (NewMax*-1);
	}
	if (NewMin >= 0) {
		if (NewMax >= 0) NewRange = NewMax - NewMin;
		if (NewMax < 0) NewRange = NewMin + (NewMax*-1);
	}

	return (long double)NewValue = ((OldValue / OldRange) * NewRange) + NewMin;
}

void DX9_Window::DrawString(const std::vector<int> &Pos, const std::vector<int> &Size, const std::string &Text, const D3DCOLOR &Color) {
	D3DRECT Rect = { Pos[0], Pos[1], (Pos[0] + Size[0]), (Pos[1] + Size[1]) };
	this->D3DFont->DrawTextA(NULL, Text.c_str(), -1, (RECT*)&Rect, NULL, Color);
}

void DX9_Window::DisplayFPSCounter(std::vector<int> Pos, D3DCOLOR Color) {
	this->FPS.FramesRendered++;
	this->FPS.CurrentTime = GetTickCount64();
	this->FPS.ElapsedTime = this->FPS.CurrentTime - this->FPS.LastTime;

	if (this->FPS.ElapsedTime >= 1000) {
		this->FPS.CurrentFPS = std::to_string(int(1000.0 * 1000.0 / this->FPS.FrameTime)) + " FPS";
		this->FPS.FramesRendered = 0;
		this->FPS.LastTime = this->FPS.CurrentTime;
	}

	this->DrawString(Pos, std::vector<int>{150, 20}, this->FPS.CurrentFPS, Color);
}

void DX9_Window::DrawKeyboard(const std::vector<int> &Pos, std::vector<bool> &KBStruct) {
	std::vector<int> TempPos = Pos;
	std::vector<int> TempSize = { 50, 50 };

	//a
	if (KBStruct.at(0x41)) {
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 128, 128, 128));
		this->DrawString(TempPos, TempSize, "A", D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	else if (!KBStruct.at(0x41)) {
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 255, 255, 255));
		this->DrawString(TempPos, TempSize, "A", D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	//s
	if (KBStruct.at(0x53)) {
		TempPos.at(0) = Pos.at(0) + 1 * (TempSize.at(0));
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 128, 128, 128));
		this->DrawString(TempPos, TempSize, "S", D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	else if (!KBStruct.at(0x53)) {
		TempPos.at(0) = Pos.at(0) + 1 * (TempSize.at(0));
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 255, 255, 255));
		this->DrawString(TempPos, TempSize, "S", D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	//w
	if (KBStruct.at(0x57)) {
		TempPos.at(0) = Pos.at(0) + 1 * (TempSize.at(0));
		TempPos.at(1) = Pos.at(1) - 1 * (TempSize.at(1));
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 128, 128, 128));
		this->DrawString(TempPos, TempSize, "W", D3DCOLOR_ARGB(255, 0, 0, 0));
		TempPos.at(1) = Pos.at(1);
	}
	else if (!KBStruct.at(0x57)) {
		TempPos.at(0) = Pos.at(0) + 1 * (TempSize.at(0));
		TempPos.at(1) = Pos.at(1) - 1 * (TempSize.at(1));
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 255, 255, 255));
		this->DrawString(TempPos, TempSize, "W", D3DCOLOR_ARGB(255, 0, 0, 0));
		TempPos.at(1) = Pos.at(1);
	}
	//d
	if (KBStruct.at(0x44)) {
		TempPos.at(0) = Pos.at(0) + 2 * (TempSize.at(0));
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 128, 128, 128));
		this->DrawString(TempPos, TempSize, "D", D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	else if (!KBStruct.at(0x44)) {
		TempPos.at(0) = Pos.at(0) + 2 * (TempSize.at(0));
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 255, 255, 255));
		this->DrawString(TempPos, TempSize, "D", D3DCOLOR_ARGB(255, 0, 0, 0));
	}
	//space
	if (KBStruct.at(VK_SPACE)) {
		TempPos.at(0) = Pos.at(0);
		TempPos.at(1) = Pos.at(1) + 1 * (TempSize.at(1));
		TempSize.at(0) *= 3;
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 128, 128, 128));
		this->DrawString(TempPos, TempSize, "SPACE", D3DCOLOR_ARGB(255, 0, 0, 0));
		TempSize.at(0) /= 3;
		TempPos.at(1) = Pos.at(1);
	}
	else if (!KBStruct.at(VK_SPACE)) {
		TempPos.at(0) = Pos.at(0);
		TempPos.at(1) = Pos.at(1) + 1 * (TempSize.at(1));
		TempSize.at(0) *= 3;
		this->DrawBox(TempPos, TempSize, D3DCOLOR_ARGB(255, 200, 200, 200), false);
		this->FillRect(TempPos, TempSize, D3DCOLOR_ARGB(255, 255, 255, 255));
		this->DrawString(TempPos, TempSize, "SPACE", D3DCOLOR_ARGB(255, 0, 0, 0));
		TempSize.at(0) /= 3;
		TempPos.at(1) = Pos.at(1);
	}


}
void DX9_Window::DrawBox(const std::vector<int> &Pos, const std::vector<int> &Size, const D3DCOLOR &Color, bool Centered) {
	//Pos.at(0) -= Size.at(0) / 2;
	//y -= y / 2;

	std::vector<D3DXVECTOR2> Points(5);

	if (Centered) {
		Points.at(0) = D3DXVECTOR2(Pos.at(0) - (Size.at(0) / 2), Pos.at(1) - (Size.at(1) / 2));
		Points.at(1) = D3DXVECTOR2(Pos.at(0) - (Size.at(0) / 2) + Size.at(0), Pos.at(1) - (Size.at(1) / 2));
		Points.at(2) = D3DXVECTOR2(Pos.at(0) - (Size.at(0) / 2) + Size.at(0), Pos.at(1) - (Size.at(1) / 2) + Size.at(1));
		Points.at(3) = D3DXVECTOR2(Pos.at(0) - (Size.at(0) / 2), Pos.at(1) - (Size.at(1) / 2) + Size.at(1));
		Points.at(4) = D3DXVECTOR2(Pos.at(0) - (Size.at(0) / 2), Pos.at(1) - (Size.at(1) / 2));
	}
	else {
		Points.at(0) = D3DXVECTOR2(Pos.at(0), Pos.at(1));
		Points.at(1) = D3DXVECTOR2(Pos.at(0) + Size.at(0), Pos.at(1));
		Points.at(2) = D3DXVECTOR2(Pos.at(0) + Size.at(0), Pos.at(1) + Size.at(1));
		Points.at(3) = D3DXVECTOR2(Pos.at(0), Pos.at(1) + Size.at(1));
		Points.at(4) = D3DXVECTOR2(Pos.at(0), Pos.at(1));
	}


	this->D3DXLine->Draw(&Points.front(), 5, Color);

}
