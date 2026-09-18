cmake_minimum_required(VERSION 3.22)
project(plugin_cpp LANGUAGES CXX)

add_library(plugin_cpp STATIC
    src/plugin_cpp.cpp
    src/plugin_cpp_entity.cpp
    src/plugin_cpp_ui.cpp
    src/plugin_cpp_item.cpp
    src/plugin_cpp_world.cpp
    src/plugin_cpp_rng.cpp
)

execute_process(
    COMMAND git describe --tags --always --dirty
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/version.hpp
    @ONLY
)

target_include_directories(plugin_cpp PUBLIC
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../game
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

target_link_libraries(plugin_cpp PUBLIC
    SDL2
)