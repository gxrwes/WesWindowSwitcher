cmake_minimum_required(VERSION 3.15)
project(wws LANGUAGES CXX)

# We’ll use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake where to find our headers
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/vendor/imgui
  ${PROJECT_SOURCE_DIR}/vendor/imgui/backends
)

# Pull in the src/ subdirectory
add_subdirectory(src)
