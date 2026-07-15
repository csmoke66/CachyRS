cmake_minimum_required(VERSION 3.22)
project(api LANGUAGES CXX)

add_library(api SHARED
    src/test.cpp
)

target_include_directories(api PUBLIC 
    include
)

find_package(OpenGL REQUIRED COMPONENTS EGL)
find_package(PkgConfig REQUIRED)

target_link_libraries(api PUBLIC mod)