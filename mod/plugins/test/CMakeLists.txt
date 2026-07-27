cmake_minimum_required(VERSION 3.22)
project(test-plugin LANGUAGES CXX)

add_library(test-plugin SHARED
    src/main.cpp
)

target_include_directories(test-plugin PUBLIC 
    include
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_link_libraries(test-plugin PRIVATE 
    mod
)