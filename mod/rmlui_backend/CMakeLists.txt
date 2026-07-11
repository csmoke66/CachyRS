cmake_minimum_required(VERSION 3.22)
project(rmlui_backend LANGUAGES CXX)

add_library(rmlui_backend STATIC
    src/RmlUi_Backend_SDL_GL3.cpp
    src/RmlUi_Renderer_GL3.cpp
    src/RmlUi_Platform_SDL.cpp
)

set_property(TARGET rmlui_backend PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(rmlui_backend PUBLIC include)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
FetchContent_Declare(
        rmlui
        GIT_REPOSITORY https://github.com/mikke89/RmlUi
)

FetchContent_MakeAvailable(rmlui)

find_package(OpenGL REQUIRED COMPONENTS 
    EGL
    )
    
    
target_link_libraries(rmlui_backend PRIVATE 
    GL 
    EGL 
    imgui
    rmlui
    SDL2
    SDL2_image
)