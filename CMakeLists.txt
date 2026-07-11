cmake_minimum_required(VERSION 3.25)
project(CachyRS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

find_package(FakeIt CONFIG REQUIRED)

add_subdirectory(injector)
add_subdirectory(mod)
add_subdirectory(updater)