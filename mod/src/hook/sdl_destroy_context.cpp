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
    void SdlDestroyContextHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);

        trampoline((SDL_GLContext)CPU_FIRST_ARG(cpu_state));
    }
}
