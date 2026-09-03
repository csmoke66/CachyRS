#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>
#include <dlfcn.h>
#include <link.h>

namespace crs
{
  void EglInitHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);
    cpu_state->rax = static_cast<uint64_t>(trampoline(
        reinterpret_cast<EGLDisplay>(CPU_FIRST_ARG(cpu_state)),
        reinterpret_cast<EGLint *>(CPU_SECOND_ARG(cpu_state)),
        reinterpret_cast<EGLint *>(CPU_THIRD_ARG(cpu_state))));
  }
} // namespace crs
