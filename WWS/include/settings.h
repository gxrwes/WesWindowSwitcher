// === src/settings.h ===
#pragma once

#include <windows.h>

// Your hotkey settings
struct HotkeyConfig {
    UINT initiator;       // e.g. VK_LMENU
    UINT modifier;        // e.g. VK_LSHIFT
    int  tapTimeoutMs;    // e.g. 300
    int  overlayTimeoutMs;// e.g. 500
};

// Accessors
HotkeyConfig& GetSettings();
HotkeyConfig  LoadSettings();
void          SaveSettings(const HotkeyConfig& cfg);

// Helpers for your UI
const char* KeyName(UINT vk);
UINT        KeyFromName(const char* name);
