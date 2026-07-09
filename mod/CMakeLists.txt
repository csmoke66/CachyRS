cmake_minimum_required(VERSION 3.22)
project(mod LANGUAGES CXX)

add_library(mod SHARED
    src/main.cpp
    src/cachy.cpp
    src/process.cpp
)

add_subdirectory(imgui)
target_include_directories(mod PRIVATE include)

find_package(OpenGL REQUIRED COMPONENTS 
    EGL
    )

target_link_libraries(mod PRIVATE 
    GL 
    EGL 
    imgui
)