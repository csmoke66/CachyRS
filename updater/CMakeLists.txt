cmake_minimum_required(VERSION 3.22)
project(updater LANGUAGES CXX)

add_executable(updater
    src/elf.cpp
    src/main.cpp
    src/pattern.cpp
    src/updater.cpp
    src/header_compiler.cpp
)

find_package(PkgConfig REQUIRED)
pkg_check_modules(capstone REQUIRED IMPORTED_TARGET capstone)

target_include_directories(updater PRIVATE include)

target_link_libraries(updater
    PkgConfig::capstone
)