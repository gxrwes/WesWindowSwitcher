# === src/CMakeLists.txt ===

# List all of our app’s sources
set(SOURCES
    main.cpp
    hook.cpp
    win_enum.cpp
    gui.cpp
    settings.cpp
)

# Define the executable
add_executable(wws WIN32 ${SOURCES})

# Header search paths for your own code and third‑party headers
target_include_directories(wws PRIVATE
    ${PROJECT_SOURCE_DIR}/include           # your gui.h, win_enum.h, settings.h
    ${PROJECT_SOURCE_DIR}/vendor/imgui      # ImGui headers
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends
    ${PROJECT_SOURCE_DIR}/vendor/json       # nlohmann/json.hpp location
)

# Build ImGui as a static library
add_library(imgui STATIC
    ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui.cpp
    ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_draw.cpp
    ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_tables.cpp
    ${PROJECT_SOURCE_DIR}/vendor/imgui/imgui_widgets.cpp
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_win32.cpp
    ${PROJECT_SOURCE_DIR}/vendor/imgui/backends/imgui_impl_dx11.cpp
)

# Link our app against ImGui and the Windows/DX11 libs
target_link_libraries(wws PRIVATE
    imgui
    d3d11
    dxgi
    d3dcompiler
    dwmapi      # for DwmIsCompositionEnabled, DwmEnableBlurBehindWindow, etc.
    psapi       # for GetModuleBaseNameW
    user32
)
