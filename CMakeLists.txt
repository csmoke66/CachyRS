cmake_minimum_required(VERSION 3.25)
project(CachyRS LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Optimize.cmake")

# Full LTO + native CPU for this overlay and its FetchContent deps (RmlUi, ImGui).
if(CACHYRS_ENABLE_LTO AND NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_options(-O3 -ffunction-sections -fdata-sections -fomit-frame-pointer -fvisibility-inlines-hidden)
  add_link_options(-Wl,--gc-sections)
  if(CACHYRS_NATIVE)
    add_compile_options(-march=native -mtune=native)
  endif()
endif()

include(FetchContent)

find_package(FakeIt CONFIG REQUIRED)

add_subdirectory(mod)
add_subdirectory(updater)
add_subdirectory(ui-sandbox)