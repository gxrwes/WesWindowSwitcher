#pragma once
#include <windows.h>
#include <string>
#include <vector>

struct WindowInfo { HWND handle; std::wstring title; };
std::vector<WindowInfo> GetOpenWindows();

// The focus hack:
void ForceSetForegroundWindow(HWND hWnd);
