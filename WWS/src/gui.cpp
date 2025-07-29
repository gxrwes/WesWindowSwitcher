// === src/gui.cpp ===
#include "gui.h"
#include "win_enum.h"
#include <windows.h>
#include "settings.h"
#include <psapi.h>  // for GetModuleBaseNameW

#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <dxgi.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
    HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam
);

// Primary monitor size
static int g_ScreenW = 0;
static int g_ScreenH = 0;

// Overlay state
static HWND                     g_hWnd = nullptr;
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRTView = nullptr;
static bool                    g_showOverlay = false;
static bool                    showSettingsPanel = false;
static std::vector<WindowInfo> g_windows;
static int                     g_selIndex = 0;

// Hotkey options
static const UINT hotkeyOptions[] = { VK_LMENU, VK_RMENU, VK_LSHIFT, VK_RSHIFT };

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferCount = 2;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    if (D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0,
        D3D11_SDK_VERSION, &sd,
        &g_pSwapChain, &g_pd3dDevice,
        nullptr, &g_pd3dContext) != S_OK)
        return false;

    ID3D11Texture2D* pBack = nullptr;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBack));
    g_pd3dDevice->CreateRenderTargetView(pBack, nullptr, &g_mainRTView);
    pBack->Release();
    return true;
}

void CleanupDeviceD3D() {
    if (g_mainRTView) { g_mainRTView->Release();  g_mainRTView = nullptr; }
    if (g_pSwapChain) { g_pSwapChain->Release();  g_pSwapChain = nullptr; }
    if (g_pd3dContext) { g_pd3dContext->Release(); g_pd3dContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release();  g_pd3dDevice = nullptr; }
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wp, lp))
        return TRUE;
    if (msg == WM_SIZE && g_pd3dDevice && wp != SIZE_MINIMIZED) {
        CleanupDeviceD3D();
        CreateDeviceD3D(hWnd);
    }
    else if (msg == WM_DESTROY) {
        PostQuitMessage(0);
    }
    return DefWindowProcW(hWnd, msg, wp, lp);
}

bool InitializeGUI(HINSTANCE hInst) {
    LoadSettings();
    HMONITOR mon = MonitorFromWindow(nullptr, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO mi{ sizeof(mi) };
    GetMonitorInfoW(mon, &mi);
    g_ScreenW = mi.rcMonitor.right - mi.rcMonitor.left;
    g_ScreenH = mi.rcMonitor.bottom - mi.rcMonitor.top;

    WNDCLASSEXW wc{ sizeof(wc), CS_CLASSDC, WndProc, 0, 0,
                    hInst, nullptr, nullptr, nullptr, nullptr,
                    L"AltTabOverlayClass", nullptr };
    RegisterClassExW(&wc);

    g_hWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED,
        wc.lpszClassName, L"AltTabOverlay",
        WS_POPUP,
        mi.rcMonitor.left, mi.rcMonitor.top,
        g_ScreenW, g_ScreenH,
        nullptr, nullptr, hInst, nullptr
    );
    if (!g_hWnd || !CreateDeviceD3D(g_hWnd)) return false;

    IMGUI_CHECKVERSION(); ImGui::CreateContext();
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dContext);

    SetLayeredWindowAttributes(g_hWnd, RGB(0, 0, 0), 0, LWA_COLORKEY);
    ShowWindow(g_hWnd, SW_HIDE);
    return true;
}

void ShutdownGUI() {
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hWnd);
    UnregisterClassW(L"AltTabOverlayClass", GetModuleHandleW(nullptr));
}

void ShowOverlay() {
    g_windows = GetOpenWindows();
    g_selIndex = (g_windows.size() > 1 ? 1 : 0);
    g_showOverlay = true;
    ShowWindow(g_hWnd, SW_SHOW);
}

void HideOverlay() {
    g_showOverlay = false;
    ShowWindow(g_hWnd, SW_HIDE);
}

void AdvanceSelection() {
    if (!g_windows.empty())
        g_selIndex = (g_selIndex + 1) % g_windows.size();
}

void SwitchToPreviousWindow() {
    auto v = GetOpenWindows();
    if (v.size() > 1)
        ForceSetForegroundWindow(v[1].handle);
}

void RenderOverlayFrame() {
    if (!g_showOverlay && !showSettingsPanel) return;

    ImGui_ImplWin32_NewFrame();
    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();

    const float pad = 10.0f;
    const float list_w = 500.0f;
    const float gear_w = 24.0f;
    const float settings_w = showSettingsPanel ? 200.0f : 0.0f;
    const float line_h = ImGui::GetFontSize() * 1.3f;
    const float panel_h = line_h * g_windows.size() + pad * 2;
    const float extra_w = showSettingsPanel ? (settings_w + pad) : (gear_w + pad);
    const float panel_w = list_w + extra_w + pad * 2;
    ImVec2 panel_sz(panel_w, panel_h);
    ImVec2 panel_pos(
        (g_ScreenW - panel_w) * 0.5f,
        (g_ScreenH - panel_h) * 0.5f
    );

    auto dl = ImGui::GetForegroundDrawList();
    dl->AddRectFilled(panel_pos,
        ImVec2(panel_pos.x + panel_w, panel_pos.y + panel_h),
        IM_COL32(30, 30, 30, 200)
    );
    dl->AddRect(panel_pos,
        ImVec2(panel_pos.x + panel_w, panel_pos.y + panel_h),
        IM_COL32(100, 100, 255, 255), 0, 0, 2.0f
    );

    ImGui::SetNextWindowPos(panel_pos);
    ImGui::SetNextWindowSize(panel_sz);
    ImGui::Begin("Overlay", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoScrollbar);

    ImGui::BeginChild("##List", ImVec2(list_w, panel_h - pad * 2), false);
    const size_t maxTitle = 80;
    for (int i = 0; i < (int)g_windows.size(); ++i) {
        auto& w = g_windows[i];
        DWORD pid = 0;
        GetWindowThreadProcessId(w.handle, &pid);
        HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
        wchar_t exeW[MAX_PATH] = L"";
        if (hProc) {
            GetModuleBaseNameW(hProc, NULL, exeW, MAX_PATH);
            CloseHandle(hProc);
        }
        std::wstring full = w.title;
        std::wstring disp;
        size_t pos = full.find(L" - ");
        if (pos != std::wstring::npos) {
            std::wstring page = full.substr(0, pos);
            std::wstring app = full.substr(pos + 3);
            disp = app + L" - " + page;
        }
        else {
            disp = full;
        }
        if (disp.length() > maxTitle)
            disp = disp.substr(0, maxTitle - 3) + L"...";
        if (i == g_selIndex)
            ImGui::TextColored(ImVec4(0.0f, 250.0f / 255.0f, 255.0f / 255.0f, 1.0f), "> %ls", disp.c_str());
        else
            ImGui::Text("  %ls", disp.c_str());
    }
    ImGui::EndChild();

    ImGui::SetCursorScreenPos(ImVec2(
        panel_pos.x + list_w + pad,
        panel_pos.y + pad
    ));
    if (ImGui::Button("⚙", ImVec2(gear_w, gear_w)))
        showSettingsPanel = !showSettingsPanel;

    if (showSettingsPanel) {
        ImGui::SameLine();
        ImGui::BeginChild("##Settings", ImVec2(settings_w, panel_h - pad * 2), false);
        ImGui::Text("Initiator");
        ImGui::SetNextItemWidth(settings_w);
        if (ImGui::BeginCombo("##Initiator", KeyName(GetSettings().initiator))) {
            for (auto k : hotkeyOptions) {
                bool sel = (GetSettings().initiator == k);
                if (ImGui::Selectable(KeyName(k), sel)) GetSettings().initiator = k;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Modifier");
        ImGui::SetNextItemWidth(settings_w);
        if (ImGui::BeginCombo("##Modifier", KeyName(GetSettings().modifier))) {
            for (auto k : hotkeyOptions) {
                bool sel = (GetSettings().modifier == k);
                if (ImGui::Selectable(KeyName(k), sel)) GetSettings().modifier = k;
                if (sel) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        ImGui::Text("Tap Timeout (ms)");
        ImGui::SetNextItemWidth(settings_w);
        ImGui::SliderInt("##TapTimeout", &GetSettings().tapTimeoutMs, 50, 1000);

        ImGui::Text("Overlay Timeout (ms)");
        ImGui::SetNextItemWidth(settings_w);
        ImGui::SliderInt("##OverlayTimeout", &GetSettings().overlayTimeoutMs, 100, 2000);

        ImGui::Spacing();
        if (ImGui::Button("Save Settings", ImVec2(settings_w, 0))) {
            SaveSettings(GetSettings());
            showSettingsPanel = false;
        }
        ImGui::EndChild();
    }

    ImGui::End();
    ImGui::Render();
    g_pd3dContext->OMSetRenderTargets(1, &g_mainRTView, nullptr);
    float clear_col[4] = { 0,0,0,0 };
    g_pd3dContext->ClearRenderTargetView(g_mainRTView, clear_col);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    g_pSwapChain->Present(1, 0);
}

void CommitSelection() {
    if (!g_windows.empty())
        ForceSetForegroundWindow(g_windows[g_selIndex].handle);
    HideOverlay();
}
