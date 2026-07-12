#include "cachy.h"
#include "not_cachy.h"

#include <format>
#include <filesystem>

#include <unistd.h>
#include <sys/mman.h>

#include <GL/gl.h>
#include <GL/glx.h>
#include <EGL/egl.h>

#include <RmlUi_Platform_SDL.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

CachyRS RS;

void CachyRS::init_logging()
{
    log_stream = std::ofstream("/tmp/cachy-rs.txt");
}

void CachyRS::init_process_info()
{
    pi.init();
}

void CachyRS::init_imgui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    if (auto sdl_window = NRS.sdl_window())
    {
        ImGui_ImplSDL2_InitForOpenGL(sdl_window, nullptr);
    }

    auto style = &ImGui::GetStyle();
    auto colors = style->Colors;

    style->WindowBorderSize = 1.0f;
    style->ChildBorderSize = 1.0f;
    style->PopupBorderSize = 1.0f;
    style->FrameBorderSize = 1.0f;

    style->WindowRounding = 2.0f;
    style->ChildRounding = 2.0f;
    style->FrameRounding = 2.0f;
    style->PopupRounding = 2.0f;
    style->GrabRounding = 2.0f;

    colors[ImGuiCol_Text] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_TextDisabled] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
    colors[ImGuiCol_WindowBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_CheckboxSelectedBg] = ImVec4(0.15f, 0.15f, 0.15f, 0.50f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 0.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.22f, 0.22f, 0.78f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.29f, 0.29f, 0.29f, 0.78f);
    colors[ImGuiCol_Separator] = ImVec4(0.29f, 0.29f, 0.29f, 0.50f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.78f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_InputTextCursor] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.49f, 0.49f, 0.49f, 0.80f);
    colors[ImGuiCol_Tab] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_TabSelected] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_TabDimmed] = ImVec4(0.29f, 0.29f, 0.29f, 0.78f);
    colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.39f, 0.39f, 0.39f, 0.78f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.29f, 0.29f, 0.29f, 0.50f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
    colors[ImGuiCol_TextLink] = ImVec4(0.29f, 0.50f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
    colors[ImGuiCol_TreeLines] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
    colors[ImGuiCol_DragDropTargetBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_UnsavedMarker] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
    colors[ImGuiCol_NavCursor] = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

void CachyRS::flush_logs()
{
    log_stream.flush();
}

std::string CachyRS::get_configuration_dir() const
{
    return interop_get_home_directory() + std::string("/.local/share/cachy-rs/");
}

std::string CachyRS::resolve_configuration(const std::string &file) const
{
    return get_configuration_dir() + file;
}

Globals *CachyRS::get_globals() const
{
    return (Globals *)pi.game_base();
}

bool CachyRS::project_to_screen(const Vec3<float> scene, Vec2<float> *out) const
{
    auto scene_003 = NRS.scene_003();
    if (!scene_003)
    {
        return false;
    }

    auto matrix = scene_003->projection_matrix;
    auto bounds = ImGui::GetIO().DisplaySize;

    Vec3<float> clip;
    clip.x = scene.x * matrix.flat[0] + scene.y * matrix.flat[4] + scene.z * matrix.flat[8] + matrix.flat[12];
    clip.y = scene.x * matrix.flat[1] + scene.y * matrix.flat[5] + scene.z * matrix.flat[9] + matrix.flat[13];

    auto w = scene.x * matrix.flat[3] + scene.y * matrix.flat[7] + scene.z * matrix.flat[11] + matrix.flat[15];
    if (w >= 0.1f)
    {
        Vec3<float> pos;
        pos.x = clip.x / w;
        pos.y = clip.y / w;

        auto x = (bounds.x / 2 * pos.x) + (pos.x + bounds.x / 2);
        auto y = -(bounds.y / 2 * pos.y) + (pos.y + bounds.y / 2);

        *out = {x, y};
        return true;
    }

    *out = {0, 0};
    return false;
}

void CachyRS::init()
{
    init_logging();

    LOG(DEBUG, "Initializing configuration directory at " << get_configuration_dir());
    std::filesystem::create_directories(std::filesystem::path(get_configuration_dir()));

    LOG(INFO, "Initializing process info...");
    init_process_info();

    LOG(INFO, "Initializing ImGui...");
    init_imgui();

    LOG(INFO, "Initializing UI...");
    ui = new RmlUserInterface();

    LOG(INFO, "Initializing capstone...");
    asm_init();

    LOG(INFO, "Placing hooks...");
    hook_manager = new HookManager(&pi);
    hook_manager->iat("swap_buffers", "eglSwapBuffers", (Hook<void*>*)&hook_egl_swap_buffers);
    hook_manager->iat("poll_event", "SDL_PollEvent", (Hook<void*>*)&hook_sdl_poll_event);
    hook_manager->x86("menu_execute", &get_globals()->menu_execute, (Hook<void *> *)&hook_menu_execute);
}
