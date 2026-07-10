cmake_minimum_required(VERSION 3.22)
project(updater LANGUAGES CXX)

add_executable(updater
    src/main.cpp
)

target_include_directories(updater PRIVATE include)

target_link_libraries(updater)