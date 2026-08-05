cmake_minimum_required(VERSION 3.22)
project(menu-entry-swapper LANGUAGES CXX)

add_library(menu-entry-swapper SHARED
    src/main.cpp
)

target_include_directories(menu-entry-swapper PUBLIC 
    include
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_link_libraries(menu-entry-swapper PRIVATE 
    mod
)