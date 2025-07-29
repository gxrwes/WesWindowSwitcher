# WWS – Windows Window Switcher

**A modern, customizable Alt+Tab replacement overlay for Windows power users.**

![screenshot-placeholder](docs/screenshot.png)

---

## Overview

**WWS** is an open-source, keyboard-driven task switcher for Windows, offering a beautiful overlay, flexible hotkeys, and behavior inspired by the built-in Alt+Tab—but with more control.

WWS is built for power users who want a snappy, low-latency way to jump between windows with just the keyboard. The project features a modern overlay interface powered by [Dear ImGui](https://github.com/ocornut/imgui), an immediate mode GUI library that enables the fast, responsive, and visually appealing switcher panel. For configuration and settings, WWS uses the excellent [nlohmann/json](https://github.com/nlohmann/json) header-only C++ JSON library (`json.hpp`), allowing user preferences to be easily stored, modified, and reloaded.

- **Fast window switching:** Overlay appears instantly, lets you quickly select and jump to any open window.
- **Customizable hotkeys:** Change the initiator (default: `Left Alt`) and modifier (`Left Shift`) via in-app settings.
- **UI:** ImGui-based overlay, dark theme, resizable, and always-on-top.
- **Portable and easy to build:** Only depends on ImGui and nlohmann/json as external libraries.
- **No window state loss:** Windows retain maximized/minimized/normal state, just like native Alt+Tab.

> **Credits:**  
> GUI overlay powered by [Dear ImGui](https://github.com/ocornut/imgui)  
> Settings powered by [nlohmann/json](https://github.com/nlohmann/json) (`json.hpp`)


---

## Features

- **Switch instantly** to the previous window with a tap.
- **Overlay mode**: Hold the initiator+modifier to pop up the overlay and cycle/select any window.
- **Quick-select:** Tap the modifier repeatedly for N-th recent window (configurable).
- **Settings panel** for hotkeys, timeouts, and overlay behavior.
- **Runs in the background** with minimal resource usage.

---

## Hotkeys

| Action                        | Default Keys             |
|-------------------------------|--------------------------|
| Quick switch to previous      | `Left Alt` tap           |
| Open overlay                  | Hold `Left Alt` + `Left Shift` |
| Cycle in overlay              | Press `Left Shift` again |
| Confirm selection             | Release `Left Alt`       |
| Cancel overlay                | Release `Left Shift`     |

You can configure all hotkeys and timeouts in the overlay’s settings ⚙ panel.

---

## Building

### **Requirements**

- Windows (tested on Windows 10/11)
- Visual Studio 2019+ **or** MinGW-w64 toolchain
- CMake 3.15+
- Git

### **Clone and Build**

```sh
git clone --recurse-submodules https://your.repo.url/wws.git
cd wws
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019"   # or use MinGW Makefiles if you prefer
cmake --build . --config Release

```

## Exiting
Just kill it in the taskmanager