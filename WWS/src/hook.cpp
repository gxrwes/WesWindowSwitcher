// === src/hook.cpp ===
#include "hook.h"
#include "win_enum.h"          // for GetOpenWindows()
#include <windows.h>
#include <functional>
#include <chrono>
#include <atomic>
#include <cstdio>

static void DebugLog(const char* fmt, ...) {
    char buf[256];
    va_list ap; va_start(ap, fmt);
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    OutputDebugStringA("WWS-HOOK: ");
    OutputDebugStringA(buf);
    OutputDebugStringA("\n");
}

static bool IsKey(UINT vk, UINT cfg) {
    switch (cfg) {
    case VK_MENU:    return vk == VK_LMENU || vk == VK_RMENU;
    case VK_SHIFT:   return vk == VK_LSHIFT || vk == VK_RSHIFT;
    default:         return vk == cfg;
    }
}

static HHOOK                   g_hHook = nullptr;
static HotkeyConfig           g_cfg;
static std::function<void()>  g_onTap;
static std::function<void()>  g_onHoldStart;
static std::function<void()>  g_onCycle;
static std::function<void()>  g_onCancel;
static std::function<void()>  g_onCommit;

// state
static bool                    g_initiatorDown = false;
static bool                    g_holdTriggered = false;
static int                     g_tapCount = 0;
static std::chrono::steady_clock::time_point g_lastShiftUpTime;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode != HC_ACTION)
        return CallNextHookEx(g_hHook, nCode, wParam, lParam);
    auto* kbd = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
    UINT vk = kbd->vkCode;
    bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
    bool up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

    //DebugLog("Event %s vk=0x%02X", down ? "DOWN" : up ? "UP" : "??", vk);

    // --- ALT down: start fresh ---
    if (down && !g_initiatorDown && IsKey(vk, g_cfg.initiator)) {
        //DebugLog("→ ALT down");
        g_initiatorDown = true;
        g_holdTriggered = false;
        g_tapCount = 0;
    }
    // --- SHIFT down: cycle if overlay is already up ---
    else if (down && g_initiatorDown && IsKey(vk, g_cfg.modifier)) {
        if (g_holdTriggered) {
            //DebugLog("→ SHIFT down (cycle)");
            g_onCycle();
        }
    }
    // --- SHIFT up: either count a tap or show overlay if first long hold ---
    else if (up && g_initiatorDown && IsKey(vk, g_cfg.modifier)) {
        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - g_lastShiftUpTime).count();
        g_lastShiftUpTime = now;

        // first time: decide hold vs. initial tap
        if (!g_holdTriggered && g_tapCount == 0) {
            // measure how long the SHIFT was held (approximate)
            static auto lastShiftDownTime = now;
            // if you need accurate hold time, capture shift-down time too
            if (delta > g_cfg.tapTimeoutMs) {
                //DebugLog("→ initial long hold → onHoldStart");
                g_holdTriggered = true;
                g_onHoldStart();
            }
            else {
                g_tapCount = 1;
                //DebugLog("→ quick-select tap #%d", g_tapCount);
            }
        }
        // after first decision, any further SHIFT-ups always increment taps
        else if (!g_holdTriggered) {
            g_tapCount++;
            //DebugLog("→ quick-select tap #%d", g_tapCount);
        }
        // if holdTriggered, ignore shift-ups here (we cycle on shift-down)
    }
    // --- ALT up: finalize action ---
    else if (up && g_initiatorDown && IsKey(vk, g_cfg.initiator)) {
        auto now = std::chrono::steady_clock::now();
        auto sinceLastShift = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - g_lastShiftUpTime).count();

        //DebugLog("→ ALT up; holdTriggered=%d tapCount=%d sinceLastShift=%lldms",            (int)g_holdTriggered, g_tapCount, (long long)sinceLastShift);

        // if you tapped but then held ALT beyond overlayTimeout, force overlay
        if (!g_holdTriggered && g_tapCount > 0 &&
            sinceLastShift > g_cfg.overlayTimeoutMs)
        {
            //DebugLog("→ delayed overlay → onHoldStart");
            g_holdTriggered = true;
            g_onHoldStart();
        }

        if (g_holdTriggered) {
            //DebugLog("→ commit overlay");
            g_onCommit();
        }
        else if (g_tapCount > 0) {
            if (g_tapCount == 1) {
                //DebugLog("→ single tap → onTap");
                g_onTap();
            }
            else {
                //DebugLog("→ multi tap → select #%d", g_tapCount);
                auto wins = GetOpenWindows();
                int idx = g_tapCount; // *** +1 from prior logic ***
                if (idx >= 0 && idx < (int)wins.size()) {
                    ForceSetForegroundWindow(wins[idx].handle);
                }
                else {
                    //DebugLog("→ index out-of-range");
                }
            }
        }
        else {
            // DebugLog("→ no action → onCancel");
            g_onCancel();
        }

        // reset
        g_initiatorDown = false;
        g_holdTriggered = false;
        g_tapCount = 0;
    }

    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

void InstallHook(const HotkeyConfig& cfg,
    std::function<void()> onTap,
    std::function<void()> onHoldStart,
    std::function<void()> onCycle,
    std::function<void()> onCancel,
    std::function<void()> onCommit)
{
    g_cfg = cfg;
    g_onTap = std::move(onTap);
    g_onHoldStart = std::move(onHoldStart);
    g_onCycle = std::move(onCycle);
    g_onCancel = std::move(onCancel);
    g_onCommit = std::move(onCommit);

    //DebugLog("Installing hook (Alt=0x%02X, Shift=0x%02X, tapTimeout=%dms, overlayTimeout=%dms)",        cfg.initiator, cfg.modifier, cfg.tapTimeoutMs, cfg.overlayTimeoutMs);
    g_hHook = SetWindowsHookExW(
        WH_KEYBOARD_LL, LowLevelKeyboardProc,
        GetModuleHandle(nullptr), 0
    );
    //DebugLog(g_hHook ? "Hook installed" : "Hook FAILED");
}

void UninstallHook() {
    //DebugLog("Uninstalling hook");
    if (g_hHook) {
        UnhookWindowsHookEx(g_hHook);
        g_hHook = nullptr;
        //DebugLog("Hook removed");
    }
}
