// === src/win_enum.cpp ===
#include "win_enum.h"
#include <windows.h>
#include <algorithm>
#include <cwctype>  // for iswspace()

static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    // must be visible & not minimized
    if (!IsWindowVisible(hwnd) || IsIconic(hwnd))
        return TRUE;

    // skip tool windows
    LONG ex = GetWindowLongW(hwnd, GWL_EXSTYLE);
    if (ex & WS_EX_TOOLWINDOW)
        return TRUE;

    // skip owned windows; only top‑level
    if (GetAncestor(hwnd, GA_ROOTOWNER) != hwnd)
        return TRUE;

    // must have a caption style (real window)
    LONG st = GetWindowLongW(hwnd, GWL_STYLE);
    if (!(st & WS_CAPTION))
        return TRUE;

    // require a non‑empty title
    int len = GetWindowTextLengthW(hwnd);
    if (len == 0)
        return TRUE;

    std::wstring title(len, L' ');
    GetWindowTextW(hwnd, &title[0], len + 1);
    title.resize(len);

    // skip all‑whitespace titles
    if (std::all_of(title.begin(), title.end(),
        [](wchar_t c) { return iswspace(c) != 0; }))
        return TRUE;

    auto* vec = reinterpret_cast<std::vector<WindowInfo>*>(lParam);
    vec->push_back({ hwnd, title });
    return TRUE;
}

std::vector<WindowInfo> GetOpenWindows() {
    std::vector<WindowInfo> r;
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(&r));
    return r;
}

void ForceSetForegroundWindow(HWND hWnd) {
    // Get ID of foreground window thread and this thread
    DWORD fgThread = GetWindowThreadProcessId(GetForegroundWindow(), nullptr);
    DWORD curThread = GetCurrentThreadId();
    // Attach threads
    AttachThreadInput(fgThread, curThread, TRUE);

    // Only restore if minimized, otherwise keep current state (preserves maximized!)
    if (IsIconic(hWnd)) {
        ShowWindow(hWnd, SW_RESTORE);
    }

    BringWindowToTop(hWnd);
    SetForegroundWindow(hWnd);

    // Detach
    AttachThreadInput(fgThread, curThread, FALSE);
}

