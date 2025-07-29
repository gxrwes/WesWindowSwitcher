// === src/gui.h ===
#pragma once

#include <windows.h>
#include "win_enum.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <vector>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

bool InitializeGUI(HINSTANCE hInstance);
void ShutdownGUI();
void ShowOverlay();
void HideOverlay();
void AdvanceSelection();
void SwitchToPreviousWindow();
void RenderOverlayFrame();
void CommitSelection();
