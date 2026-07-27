#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>

#include <cmath>
#include <wayland-egl.h>

namespace crs
{
    void EglCreateWindowSurfaceHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);
        cpu_state->rax = (uint64_t)trampoline(
            (EGLDisplay)CPU_FIRST_ARG(cpu_state),
            (EGLConfig)CPU_SECOND_ARG(cpu_state),
            (NativeWindowType)CPU_THIRD_ARG(cpu_state),
            (EGLint *)CPU_FOURTH_ARG(cpu_state));

        if (!cpu_state->rax)
        {
            LOG(EglCreateWindowSurfaceHook, "Failed to create surface " << std::hex << eglGetError());
        }
    }
}
