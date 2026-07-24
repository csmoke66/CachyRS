cmake_minimum_required(VERSION 3.22)
project(mod LANGUAGES CXX)

add_library(mod SHARED
    src/log.cpp
    src/cachy.cpp
    src/not_cachy.cpp
    src/util.cpp
    src/event_bus.cpp
    src/interop.cpp
    src/process_linux.cpp

    src/dom/dom.cpp
    src/dom/cachy_dom.cpp
    src/dom/item_containers.cpp
    src/dom/npcs.cpp
    src/dom/players.cpp
    
    src/ui/ui.cpp
    src/ui/rml_ui.cpp
    src/ui/developer.cpp
    
    src/reversed.cpp
    
    src/hook/hook.cpp
    src/hook/hook_linux.cpp
    src/hook/hook_manager.cpp
    src/hook/hook_egl_swap_buffers.cpp
    src/hook/hook_menu_execute.cpp
    src/hook/hook_sdl_poll_event.cpp
    src/hook/hook_render_widget.cpp
    src/hook/hook_engine_tick.cpp

    # keep this last, since we run our code in a static initializer
    # putting it last puts it last in the initialization list
    src/main.cpp
)

target_include_directories(mod PUBLIC 
    include
    game
)

execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/version.hpp
    @ONLY
)

target_include_directories(mod PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_subdirectory(imgui)
add_subdirectory(rmlui_backend)

FetchContent_Declare(
        rmlui
        GIT_REPOSITORY https://github.com/mikke89/RmlUi
)

FetchContent_MakeAvailable(rmlui)

find_package(OpenGL REQUIRED COMPONENTS EGL)
find_package(PkgConfig REQUIRED)

pkg_check_modules(capstone REQUIRED IMPORTED_TARGET capstone)

target_link_libraries(mod PRIVATE 
    GL 
    EGL 
    imgui
    rmlui
    SDL2
    SDL2_image
    rmlui_backend
    PkgConfig::capstone
)