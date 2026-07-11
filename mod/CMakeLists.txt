cmake_minimum_required(VERSION 3.22)
project(mod LANGUAGES CXX)

add_library(mod SHARED
    src/main.cpp
    src/cachy.cpp
    src/not_cachy.cpp
    src/process.cpp
    src/cachy_rmlui.cpp
    src/interop.cpp
)

add_subdirectory(imgui)
add_subdirectory(rmlui_backend)
target_include_directories(mod PRIVATE include)

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
FetchContent_Declare(
        rmlui
        GIT_REPOSITORY https://github.com/mikke89/RmlUi
)

FetchContent_MakeAvailable(rmlui)

find_package(OpenGL REQUIRED COMPONENTS 
    EGL
    )
    
    
target_link_libraries(mod PRIVATE 
    GL 
    EGL 
    imgui
    rmlui
    SDL2
    SDL2_image
    rmlui_backend
)