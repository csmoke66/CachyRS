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

#include "cachy_rmlui.h"

CachyRS RS;

template <typename FN>
void iterate_entities(WorldNode *node, FN fn)
{
    if (auto entity = node->entity)
    {
        fn(entity);
    }

    for (auto c = node->children.begin; c != node->children.end; c++)
    {
        iterate_entities(*c, fn);
    }
}

uint64_t hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
#define FLUSH_GL_ERRORS() while (glGetError() != GL_NO_ERROR)

    static EGLint cached_width, cached_height;
    static bool is_first_run = true;

    auto& io = ImGui::GetIO();

    EGLint width, height;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (is_first_run)
    {
        ImGui_ImplOpenGL3_Init("#version 330"); // TODO FIXME check error
        RS.init_rmlui(width, height);

        // flush OpenGL errors so they don't propagate to rmlui
        FLUSH_GL_ERRORS();

        is_first_run = false;
    }

    if (cached_width != width || cached_height != height)
    {
        cached_width = width;
        cached_height = height;

        io.DisplaySize = ImVec2(width, height);
        io.DeltaTime = 1.0f / 60.0f;
    }

    // clang-format off
    RS.event_ring_buffer.process([](auto& event)
    {
        ImGui_ImplSDL2_ProcessEvent(&event);
        RS.rmlui_wants_input = !RmlSDL::InputEventHandler(RS.rmlui_context, NRS.sdl_window(), event);
    });
    // clang-format on

    { /* imgui */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();
        if (ImGui::Begin("Developer"))
        {
            if (ImGui::Button("Reload UI"))
            {
                RS.reload_ui();
            }
        }

        if (RS.player_overlay_on)
        {
            if (auto root = NRS.root_node())
            {
                // clang-format off
                iterate_entities(root, [](Entity *entity) 
                {
                    if (entity->type == EntityType::player)
                    {
                        Vec2<float> screen_pos;
                        if (RS.project_to_screen(entity->position, &screen_pos))
                        {
                            auto bg = ImGui::GetBackgroundDrawList();
                            bg->AddText(ImVec2(screen_pos.x, screen_pos.y), IM_COL32(255, 0, 0, 170), entity->name.c_str());
                        }
                    }
                });
                // clang-format on
            }
        }
        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // flush OpenGL errors so they don't propagate to rmlui
    FLUSH_GL_ERRORS();

    { /* rmlui */
        RS.update_rmlui();
        RS.rmlui_context->Update();

        Backend::BeginFrame();
        RS.rmlui_context->Render();
        Backend::PresentFrame();
    }

    return RS.real_eglSwapBuffers(dpy, surface);
}

static int hook_SDL_PollEvent(SDL_Event *event)
{
    auto ret = RS.real_SDL_PollEvent(event);
    if (ret)
    {
        RS.event_ring_buffer.push(*event);

        auto steal_processing = false;
        auto &io = ImGui::GetIO();
        if (io.WantCaptureMouse || io.WantCaptureKeyboard)
        {
            steal_processing = true;
        }

        if (RS.rmlui_wants_input)
        {
            steal_processing = true;
        }

        if (steal_processing)
        {
            while (RS.real_SDL_PollEvent(event))
            {
                RS.event_ring_buffer.push(*event);
            }

            return 0;
        }
    }

    return ret;
}

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

std::string CachyRS::get_configuration_dir()
{
    return interop_get_home_directory() + std::string("/.local/share/cachy-rs/");
}

std::string CachyRS::resolve_configuration(const std::string &file)
{
    return get_configuration_dir() + file;
}

void CachyRS::hook_import(const std::string &symbol, void **original, void *target)
{
    ImportedFunction fn;
    if (pi.find_import(symbol, &fn))
    {
        auto page = (Elf64_Addr)fn.addr & ~(getpagesize() - 1);

        mprotect((void *)page, getpagesize(), PROT_READ | PROT_WRITE);
        if (original)
        {
            *original = (void *)*fn.addr;
        }
        *fn.addr = (Elf64_Addr)target;
        mprotect((void *)page, getpagesize(), PROT_READ);
    }
}

Globals *CachyRS::get_globals()
{
    return (Globals *)pi.game_base();
}

bool CachyRS::project_to_screen(Vec3<float> scene, Vec2<float> *out)
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

    LOG(INFO, "Placing hooks...");
    hook_import("eglSwapBuffers", (void **)&RS.real_eglSwapBuffers, (void *)&hook_eglSwapBuffers);
    hook_import("SDL_PollEvent", (void **)&RS.real_SDL_PollEvent, (void *)&hook_SDL_PollEvent);
}
