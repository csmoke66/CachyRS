#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>
#include <SDL2/SDL_syswm.h>

#include <cmath>
#include <wayland-egl.h>

namespace crs
{
  void SdlGetWindowWMInfoHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    cpu_state->rax = static_cast<uint64_t>(trampoline(
        reinterpret_cast<SDL_Window *>(CPU_FIRST_ARG(cpu_state)),
        reinterpret_cast<SDL_SysWMinfo *>(CPU_SECOND_ARG(cpu_state))));

    this->info = *(SDL_SysWMinfo *)CPU_SECOND_ARG(cpu_state);
  }
} // namespace crs
