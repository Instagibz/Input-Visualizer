#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <atomic>

#pragma comment(lib, "winmm.lib")

#include "DX9Window.h"

HHOOK _hookKB;
HHOOK _hookMS;
KBDLLHOOKSTRUCT kbdStruct;
std::vector<bool> KBState(256);

LRESULT __stdcall KBHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode >= 0) {
		kbdStruct = *((KBDLLHOOKSTRUCT*)lParam);

		if (wParam == WM_KEYDOWN) {
			for (int i = 0; i < 256; i++) {
				if (kbdStruct.vkCode == i) {
					KBState.at(i) = true;
					break;
				}
			}
		}
		else if (wParam == WM_KEYUP) {
			for (int i = 0; i < 256; i++) {
				if (kbdStruct.vkCode == i) {
					KBState.at(i) = false;
					break;
				}
			}
		}
	}

	return CallNextHookEx(_hookKB, nCode, wParam, lParam);
}
LRESULT __stdcall MSHookCallback(int nCode, WPARAM wParam, LPARAM lParam) {

	if (nCode >= 0) {
		if (wParam == WM_LBUTTONDOWN) { KBState.at(1) = true; }
		else if (wParam == WM_LBUTTONUP) { KBState.at(1) = false; }

		if (wParam == WM_RBUTTONDOWN) { KBState.at(2) = true; }
		else if (wParam == WM_RBUTTONUP) { KBState.at(2) = false; }

		if (wParam == WM_MBUTTONDOWN) { KBState.at(3) = true; }
		else if (wParam == WM_MBUTTONUP) { KBState.at(3) = false; }

		if (wParam == WM_XBUTTONDOWN) { KBState.at(4) = true; }
		else if (wParam == WM_XBUTTONUP) { KBState.at(4) = false; }
	}

	return CallNextHookEx(_hookMS, nCode, wParam, lParam);
}


void MessageLoop(std::atomic<bool> & Shutdown) {
	_hookKB = SetWindowsHookEx(WH_KEYBOARD_LL, KBHookCallback, NULL, 0);
	_hookMS = SetWindowsHookEx(WH_MOUSE_LL, MSHookCallback, NULL, 0);
	MSG Message;
	UINT_PTR MessageTimer = SetTimer(NULL, NULL, 250, NULL);

	while (GetMessage(&Message, NULL, NULL, NULL) && !Shutdown) { DispatchMessage(&Message); }

	KillTimer(NULL, MessageTimer);
	UnhookWindowsHookEx(_hookKB);
	UnhookWindowsHookEx(_hookMS);
}
static DWORD LastFrameTime = 0;

void LimitFPS(int FPS) {
	DWORD currentTime = timeGetTime();

	if ((currentTime - LastFrameTime) < (2000 / FPS))
	{
		DWORD SleepAmount = currentTime - LastFrameTime;

		if (SleepAmount >= 1) { Sleep(SleepAmount); }
		else 	while (timeGetTime() - LastFrameTime > 0) {
			Sleep(0);
		};

	}
	LastFrameTime = currentTime;
}

int main() {
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	MSG Message;
	std::vector<int> Dimensions = { 150, 150 };
	std::vector<int> Position = { 0, 0 };
	std::atomic<bool> ShutdownFlag = false;

	DX9_Window Overlay((HINSTANCE)GetWindowLongPtr(GetConsoleWindow(), GWLP_HINSTANCE), SW_SHOW, Position, Dimensions);

	std::thread MessageThread(MessageLoop, std::ref(ShutdownFlag));

	std::vector<int> BoardPos = { 0, 50 };
	double FPSLimit = 300;

	while (!GetAsyncKeyState(VK_END)) {
		Overlay.BeginScene();
		Overlay.FillRect(Position, Dimensions, D3DCOLOR_ARGB(255, 0, 255, 0));
		Overlay.DrawKeyboard(BoardPos, KBState);
		Overlay.EndScene();

		if (Overlay.FPS.FrameTime <= (1000.0 / FPSLimit)*1000.0) {
			int SleepFor = ((1000.0 / FPSLimit)*1000.0) - Overlay.FPS.FrameTime;
			std::this_thread::sleep_for(std::chrono::microseconds(SleepFor));
			Overlay.FPS.FrameTime += SleepFor;
		}


		if (PeekMessage(&Message, NULL, NULL, NULL, PM_REMOVE)) {

			if (Message.message == WM_QUIT) {
				ShutdownFlag = true;
				MessageThread.join();
				return 0;
			}
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
	}

	ShutdownFlag = true;
	MessageThread.join();

	return 0;
}