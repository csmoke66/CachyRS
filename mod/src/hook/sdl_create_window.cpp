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
  void SdlCreateWindowHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
        reinterpret_cast<char*>(CPU_FIRST_ARG(cpu_state)),
        static_cast<int>(CPU_SECOND_ARG(cpu_state)), 
        static_cast<int>(CPU_THIRD_ARG(cpu_state)),
        static_cast<int>(CPU_FOURTH_ARG(cpu_state)), 
        static_cast<int>(CPU_FIFTH_ARG(cpu_state)),
        static_cast<uint32_t>(CPU_SIXTH_ARG(cpu_state))));
  }
} // namespace crs
