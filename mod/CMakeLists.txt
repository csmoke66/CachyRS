cmake_minimum_required(VERSION 3.22)
project(mod LANGUAGES CXX)

add_library(mod SHARED
    src/log.cpp
    src/cachy.cpp
    src/not_cachy.cpp
    src/util.cpp
    src/interop.cpp
    src/process_linux.cpp
    src/plugin.cpp
    
    src/event_bus.cpp
    src/game_events.cpp
    
    src/dom/dom.cpp
    src/dom/cachy.cpp
    src/dom/item_containers.cpp
    src/dom/npcs.cpp
    src/dom/players.cpp
    src/dom/world_settings.cpp
    
    src/ui/ui.cpp
    src/ui/rml.cpp
    src/ui/developer.cpp
    
    src/reversed.cpp
    
    src/hook/hook.cpp
    src/hook/linux.cpp
    src/hook/manager.cpp
    src/hook/egl_swap_buffers.cpp
    src/hook/egl_get_display.cpp
    src/hook/egl_init.cpp
    src/hook/egl_create_window_surface.cpp
    src/hook/egl_choose_config.cpp
    src/hook/sdl_get_window_wm_info.cpp
    src/hook/sdl_create_window.cpp
    src/hook/sdl_create_context.cpp
    src/hook/sdl_destroy_context.cpp
    src/hook/menu_execute.cpp
    src/hook/sdl_poll_event.cpp
    src/hook/render_widget.cpp
    src/hook/engine_tick.cpp
    src/hook/set_var_bit.cpp
    src/hook/dl_open.cpp

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
add_subdirectory(plugins/test)

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