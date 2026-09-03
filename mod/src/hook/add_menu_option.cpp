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

    auto option_text = (const char *)CPU_SECOND_ARG(cpu_state);
    auto templ = (MenuActionTemplate *)CPU_FIFTH_ARG(cpu_state);

    trampoline(
        (void *)CPU_FIRST_ARG(cpu_state),
        option_text,
        (uint8_t *)CPU_THIRD_ARG(cpu_state),
        (int32_t)CPU_FOURTH_ARG(cpu_state),
        templ,
        (int32_t *)CPU_SIXTH_ARG(cpu_state),
        (int32_t)CPU_STACK_ARG(cpu_state, 0),
        (int32_t)CPU_STACK_ARG(cpu_state, 1),
        (int32_t)CPU_STACK_ARG(cpu_state, 2),
        (int32_t)CPU_STACK_ARG(cpu_state, 3),
        (uint8_t)CPU_STACK_ARG(cpu_state, 4),
        (uint8_t)CPU_STACK_ARG(cpu_state, 5),
        (int32_t)CPU_STACK_ARG(cpu_state, 6),
        (uint8_t)CPU_STACK_ARG(cpu_state, 7),
        (uint8_t)CPU_STACK_ARG(cpu_state, 8),
        (void *)CPU_STACK_ARG(cpu_state, 9),
        (uint8_t)CPU_STACK_ARG(cpu_state, 10),
        (int32_t)CPU_STACK_ARG(cpu_state, 11));
  }
} // namespace crs
