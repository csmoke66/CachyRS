cmake_minimum_required(VERSION 3.22)
project(plugin_cpp LANGUAGES CXX)

add_library(plugin_cpp STATIC
    src/plugin_cpp.cpp
    src/plugin_cpp_entity.cpp
    src/plugin_cpp_ui.cpp
)

target_include_directories(plugin_cpp PUBLIC include)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_link_libraries(plugin_cpp PRIVATE 
    mod
)