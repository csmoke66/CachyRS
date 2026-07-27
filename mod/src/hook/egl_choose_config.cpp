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
    void EglChooseConfigHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);
        cpu_state->rax = (uint64_t)trampoline(
            (EGLDisplay)CPU_FIRST_ARG(cpu_state),
            (EGLint *)CPU_SECOND_ARG(cpu_state),
            (EGLConfig *)CPU_THIRD_ARG(cpu_state),
            (EGLint)CPU_FOURTH_ARG(cpu_state),
            (EGLint *)CPU_FIFTH_ARG(cpu_state));

        if (!cpu_state->rax)
        {
            LOG(EglChooseConfigHook, "eglChooseConfig failed " << std::hex << eglGetError());
        }
    }
}
