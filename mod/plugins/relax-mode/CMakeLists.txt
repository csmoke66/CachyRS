cmake_minimum_required(VERSION 3.22)
project(relax-mode LANGUAGES CXX)

add_library(relax-mode SHARED
    src/main.cpp
)

target_include_directories(relax-mode PUBLIC 
    include
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_link_libraries(relax-mode PRIVATE 
    mod
    plugin_cpp
)