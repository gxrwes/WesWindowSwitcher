// === src/main.cpp ===
#include "hook.h"
#include "gui.h"
#include "win_enum.h"
#include <windows.h>
#include <exception>

inline void DebugLog(const char* msg) {
    OutputDebugStringA("WWS: ");
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int) {
    try {
        //DebugLog("Initializing GUI");
        if (!InitializeGUI(hInst)) {
            DebugLog("InitializeGUI failed");
            return 1;
        }
        // DebugLog("GUI initialized");

        HotkeyConfig cfg{ VK_LMENU, VK_LSHIFT, 230, 200 };
        //DebugLog("Installing hook");
        InstallHook(cfg,
            []() { SwitchToPreviousWindow(); },
            []() { ShowOverlay(); },
            []() { AdvanceSelection(); },
            []() { HideOverlay(); },
            []() { CommitSelection(); }   
        );

        //DebugLog("Entering loop");
        MSG msg;
        bool running = true;
        while (running) {
            // Process all pending messages
            while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
                if (msg.message == WM_QUIT) {
                    running = false;
                    break;
                }
                TranslateMessage(&msg);
                DispatchMessageW(&msg);
            }
            // Render (if needed) then sleep so we don't spin at 100%
            RenderOverlayFrame();
            Sleep(10);
        }

        //DebugLog("Cleaning up");
        UninstallHook();
        ShutdownGUI();
        return 0;
    }
    catch (const std::exception& e) {
        DebugLog(e.what());
    }
    catch (...) {
        DebugLog("Unknown exception in WinMain");
    }
    return -1;
}
