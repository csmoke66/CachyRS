#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <GL/gl.h>

#define FLUSH_GL_ERRORS()               \
    while (glGetError() != GL_NO_ERROR) \
    {                                   \
    }

namespace crs
{
    void EglSwapBuffersHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);

        auto dpy = (EGLDisplay)CPU_FIRST_ARG(cpu_state);
        auto surface = (EGLSurface)CPU_SECOND_ARG(cpu_state);

        EGLint width, height;
        eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
        eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

        if (is_first_run)
        {
            ImGui_ImplOpenGL3_Init("#version 330"); // TODO FIXME check error

            // TODO FIXME this could be unsafe
            auto globals = RS.get_globals().unwrap_unsafe();
            auto sdl_window = dref<SDL_Window *>(
                globals,
                {off(Globals, linux_001),
                 off(Linux001, linux_002),
                 off(Linux002, linux_003),
                 off(Linux003, linux_004),
                 off(Linux004, linux_005),
                 off(Linux005, sdl_window)});

            RS.ui->init(std::string(FEATURE_VERSION) + CACHYRS_VERSION, RS.get_configuration_dir(), sdl_window, width, height);

            // flush OpenGL errors so they don't propagate to rmlui
            FLUSH_GL_ERRORS();

            is_first_run = false;
        }

        if (cached_width != width || cached_height != height)
        {
            cached_width = width;
            cached_height = height;

            auto &io = ImGui::GetIO();
            io.DisplaySize = ImVec2(width, height);
            io.DeltaTime = 1.0f / 60.0f;
        }

        // clang-format off
        RS.event_ring_buffer.process([](auto& event)
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            RS.ui->process(&event);
        });

        // clang-format on

        { /* imgui */
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            if (ImGui::Begin("Developer"))
            {
                if (ImGui::Button("Reload UI"))
                {
                    RS.ui->reload();
                }
            }

            RS.developer_overlay.render();

            ImGui::End();
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        // flush OpenGL errors so they don't propagate to rmlui
        FLUSH_GL_ERRORS();

        { /* ui */
            RS.push_ui_state();
            RS.ui->render();
        }

        cpu_state->rax = (uint64_t)trampoline(dpy, surface);
    }
}