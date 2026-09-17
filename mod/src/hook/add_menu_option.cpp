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
  void AddMenuOptionHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto option_text = reinterpret_cast<const char *>(CPU_SECOND_ARG(cpu_state));
    auto templ = reinterpret_cast<MenuActionTemplate *>(CPU_FIFTH_ARG(cpu_state));

    trampoline(
        reinterpret_cast<void *>(CPU_FIRST_ARG(cpu_state)),
        option_text,
        reinterpret_cast<uint8_t *>(CPU_THIRD_ARG(cpu_state)),
        static_cast<int32_t>(CPU_FOURTH_ARG(cpu_state)),
        templ,
        reinterpret_cast<int32_t *>(CPU_SIXTH_ARG(cpu_state)),
        static_cast<int32_t>(CPU_STACK_ARG(cpu_state, 0)),
        static_cast<int32_t>(CPU_STACK_ARG(cpu_state, 1)),
        static_cast<int32_t>(CPU_STACK_ARG(cpu_state, 2)),
        static_cast<int32_t>(CPU_STACK_ARG(cpu_state, 3)),
        static_cast<uint8_t>(CPU_STACK_ARG(cpu_state, 4)),
        static_cast<uint8_t>(CPU_STACK_ARG(cpu_state, 5)),
        static_cast<int32_t>(CPU_STACK_ARG(cpu_state, 6)),
        static_cast<uint8_t>(CPU_STACK_ARG(cpu_state, 7)),
        static_cast<uint8_t>(CPU_STACK_ARG(cpu_state, 8)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 9)),
        static_cast<uint8_t>(CPU_STACK_ARG(cpu_state, 10)),
        static_cast<int32_t>(CPU_STACK_ARG(cpu_state, 11)));
  }
} // namespace crs
