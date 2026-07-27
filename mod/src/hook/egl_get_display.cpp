#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <dlfcn.h>

#include <X11/Xlib.h>

namespace crs
{
    void EglGetDisplayHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);
        cpu_state->rax = (uint64_t)trampoline(EGL_DEFAULT_DISPLAY);

        if (!cpu_state->rax)
        {
            LOG(EglGetDisplayHook, "Failed to create display " << std::hex << eglGetError());
        }
    }
}
