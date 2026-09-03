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
    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
        reinterpret_cast<EGLDisplay>(CPU_FIRST_ARG(cpu_state)),
        reinterpret_cast<EGLConfig>(CPU_SECOND_ARG(cpu_state)),
        reinterpret_cast<NativeWindowType>(CPU_THIRD_ARG(cpu_state)),
        reinterpret_cast<EGLint *>(CPU_FOURTH_ARG(cpu_state))));

    if (!cpu_state->rax)
    {
      LOG(EglCreateWindowSurfaceHook, "Failed to create surface " << std::hex << eglGetError());
    }
  }
} // namespace crs
