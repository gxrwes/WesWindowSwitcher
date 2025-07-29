// hook.h
#pragma once

#include <windows.h>
#include <functional>
#include <chrono>

// State machine for our switcher
enum class SwitcherState { Idle, TapPending, QuickSelect, Listing };
// Hotkey configuration
struct HotkeyConfig {
    UINT initiator;    // e.g. VK_MENU (Alt)
    UINT modifier;     // e.g. VK_TAB
    int tapTimeoutMs;  // e.g. 300
    int  overlayTimeoutMs;
};

// Install/uninstall the low-level keyboard hook
void InstallHook(const HotkeyConfig& cfg,
    std::function<void()> onTap,
    std::function<void()> onHoldStart,
    std::function<void()> onCycle,
    std::function<void()> onCancel,
    std::function<void()> onCommit);
void UninstallHook();
