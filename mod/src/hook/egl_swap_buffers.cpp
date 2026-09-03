#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <GL/gl.h>

#define FLUSH_GL_ERRORS()             \
  while (glGetError() != GL_NO_ERROR) \
  {                                   \
  }

namespace crs
{
  void EglSwapBuffersHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto dpy = reinterpret_cast<EGLDisplay>(CPU_FIRST_ARG(cpu_state));
    auto surface = reinterpret_cast<EGLSurface>(CPU_SECOND_ARG(cpu_state));

    auto tick_hook = RS.hook_manager->view_hook<BaseHook>("engine_tick");
    if (!tick_hook || !tick_hook->thread_id().has_value())
    {
      cpu_state->rax = static_cast<uint64_t>(trampoline(dpy, surface));
      return;
    }

    EGLint width, height;
    eglQuerySurface(dpy, surface, EGL_WIDTH, &width);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &height);

    if (is_first_run)
    {
      // TODO FIXME this could be unsafe
      auto globals = RS.get_globals().unwrap_unsafe();
      auto sdl_window = dref<SDL_Window *>(
          globals,
          { off(Globals, linux_001),
              off(Linux001, linux_002),
              off(Linux002, linux_003),
              off(Linux003, linux_004),
              off(Linux004, linux_005),
              off(Linux005, sdl_window) });

      ImGui_ImplSDL2_InitForOpenGL(sdl_window, nullptr);

      if (!ImGui_ImplOpenGL3_Init("#version 330"))
      {
        LOG(EglSwapBuffersHook, "Failed to initialize ImGui OpenGL backend");
      }

      RS.ui->init(std::string(FEATURE_VERSION) + CACHYRS_VERSION, RS.get_configuration_dir(), sdl_window, width, height);

      // flush OpenGL errors so they don't propagate to rmlui
      FLUSH_GL_ERRORS();

      auto &io = ImGui::GetIO();
      io.DeltaTime = 1.0f / 60.0f;

      is_first_run = false;
    }

    RS.ui_locked([this, width, height]()
    {
      if (cached_width != width || cached_height != height)
      {
        cached_width = width;
        cached_height = height;

        auto &io = ImGui::GetIO();
        io.DisplaySize = ImVec2(width, height);
      }

      RS.event_ring_buffer.process([](SDL_Event &event)
      {
        if (event.type == SDL_KEYDOWN &&
            event.key.keysym.scancode == SDL_SCANCODE_INSERT)
        {
          RS.ui_visible = !RS.ui_visible;
        }

        ImGui_ImplSDL2_ProcessEvent(&event);

        if (RS.ui_visible)
        {
          RS.ui->process(&event);
        }
      });

      { /* imgui */
        if (auto draw_data = ImGui::GetDrawData())
        {
          ImGui_ImplOpenGL3_RenderDrawData(draw_data);
        }
      }

      // flush OpenGL errors so they don't propagate to rmlui
      FLUSH_GL_ERRORS();

      { /* ui */
        RS.stats.push_ui_state_stopwatch.reset();
        RS.push_ui_state();
        RS.stats.push_ui_state_stopwatch.stop();

        if (RS.ui_visible)
        {
          RS.stats.render_ui_stopwatch.reset();
          RS.ui->render();
          RS.stats.render_ui_stopwatch.stop();
        }
      }

      return false;
    });

    cpu_state->rax = static_cast<uint64_t>(trampoline(dpy, surface));
  }
} // namespace crs
