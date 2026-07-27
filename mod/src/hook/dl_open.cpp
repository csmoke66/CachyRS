#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

namespace crs
{
    void DlOpenHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);

        auto name = (const char *)CPU_FIRST_ARG(cpu_state);
        LOG(DlOpenHook, "Opening " << (name ? name : "NULL"));
        
        cpu_state->rax = (uint64_t)trampoline((const char *)CPU_FIRST_ARG(cpu_state), (int)CPU_SECOND_ARG(cpu_state));
    }
}
