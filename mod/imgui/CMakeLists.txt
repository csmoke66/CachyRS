add_library(imgui STATIC
    src/imgui_draw.cpp
    src/imgui_impl_opengl3.cpp
    src/imgui_tables.cpp
    src/imgui_widgets.cpp
    src/imgui.cpp
    src/imgui_impl_sdl2.cpp
)

set_property(TARGET imgui PROPERTY POSITION_INDEPENDENT_CODE ON)
target_include_directories(imgui PUBLIC include)

find_package(OpenGL REQUIRED COMPONENTS EGL)
target_link_libraries(imgui PRIVATE GL EGL)
