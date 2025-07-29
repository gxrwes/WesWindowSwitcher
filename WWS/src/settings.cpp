// === src/settings.cpp ===
#include "settings.h"

#include <fstream>
#include <vector>
#include <cstring>           // for strcmp()
#include "json.hpp"        // single-header nlohmann/json

using json = nlohmann::json;

static HotkeyConfig g_cfg;
static const char* SETTINGS_FILE = "wws_config.json";

// Dropdown options
static const std::vector<UINT> keys = { VK_LMENU, VK_RMENU, VK_LSHIFT, VK_RSHIFT };
static const std::vector<const char*> keyNames = { "Left Alt", "Right Alt", "Left Shift", "Right Shift" };

const char* KeyName(UINT vk) {
    for (size_t i = 0; i < keys.size(); ++i)
        if (keys[i] == vk)
            return keyNames[i];
    return "Unknown";
}

UINT KeyFromName(const char* name) {
    for (size_t i = 0; i < keyNames.size(); ++i)
        if (std::strcmp(name, keyNames[i]) == 0)
            return keys[i];
    return VK_LMENU;
}

HotkeyConfig& GetSettings() {
    return g_cfg;
}

HotkeyConfig LoadSettings() {
    std::ifstream ifs(SETTINGS_FILE);
    if (ifs) {
        json j; ifs >> j;
        g_cfg.initiator = j.value("initiator", VK_LMENU);
        g_cfg.modifier = j.value("modifier", VK_LSHIFT);
        g_cfg.tapTimeoutMs = j.value("tapTimeoutMs", 300);
        g_cfg.overlayTimeoutMs = j.value("overlayTimeoutMs", 500);
    }
    else {
        g_cfg = { VK_LMENU, VK_LSHIFT, 300, 500 };
    }
    return g_cfg;
}

void SaveSettings(const HotkeyConfig& cfg) {
    json j;
    j["initiator"] = cfg.initiator;
    j["modifier"] = cfg.modifier;
    j["tapTimeoutMs"] = cfg.tapTimeoutMs;
    j["overlayTimeoutMs"] = cfg.overlayTimeoutMs;
    std::ofstream ofs(SETTINGS_FILE);
    ofs << j.dump(4);
}
