cmake_minimum_required(VERSION 3.22)
project(ui-sandbox LANGUAGES CXX)

add_executable(ui-sandbox
    main.cpp
)

target_compile_features(ui-sandbox PRIVATE cxx_std_26)

target_link_libraries(ui-sandbox PRIVATE
    crs_ui
    GL
    EGL
    SDL2
)

# Resolve rmlui assets next to the binary via symlink/copy at build time.
set(UI_SANDBOX_RMLUI_SRC "${CMAKE_SOURCE_DIR}/config/rmlui")
set(UI_SANDBOX_RMLUI_DST "$<TARGET_FILE_DIR:ui-sandbox>/rmlui")

add_custom_command(TARGET ui-sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E rm -rf "${UI_SANDBOX_RMLUI_DST}"
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${UI_SANDBOX_RMLUI_SRC}" "${UI_SANDBOX_RMLUI_DST}"
    COMMENT "Linking rmlui assets for ui-sandbox"
)

cachyrs_warnings(ui-sandbox)
