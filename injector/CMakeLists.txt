cmake_minimum_required(VERSION 3.17)
project(injector)

include_directories(${CMAKE_SOURCE_DIR})

option(STATIC_BUILD "enable static build" OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)

include(FetchContent)

FetchContent_Declare(
        zero
        GIT_REPOSITORY https://github.com/csmoke66/zero
        GIT_TAG        main 
)

FetchContent_MakeAvailable(zero)

add_executable(injector 
        src/main.cpp 
        src/tracee.cpp 
        src/executor.cpp 
        src/injector.cpp
        src/system.cpp
)

target_include_directories(injector PRIVATE include)
target_link_libraries(injector zero $<$<BOOL:${STATIC_BUILD}>:-static-pie>)