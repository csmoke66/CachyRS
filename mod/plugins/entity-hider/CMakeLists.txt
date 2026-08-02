cmake_minimum_required(VERSION 3.22)
project(entity-hider LANGUAGES CXX)

add_library(entity-hider SHARED
    src/main.cpp
)

target_include_directories(entity-hider PUBLIC 
    include
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_link_libraries(entity-hider PRIVATE 
    mod
)