#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

#include <EGL/egl.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <cstring>

namespace crs
{
  void AddChatMessageHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto name_1 = reinterpret_cast<JString *>(CPU_FIFTH_ARG(cpu_state));
    auto message = reinterpret_cast<JString *>(CPU_STACK_ARG(cpu_state, 1));
    auto channel = reinterpret_cast<JString *>(CPU_STACK_ARG(cpu_state, 3));

    if (std::strcmp(message->c_str(), "Welcome to RuneScape.") == 0)
    {
      std::strcpy(const_cast<char *>(std::strstr(message->c_str(), "RuneScape")), "CachyRS.");
    }

    auto event = NewChatMessageEvent(channel->str(), name_1->str(), message->str());
    RS.event_bus.dispatch(NewChatMessageEvent::specific_id(), &event);

    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
        reinterpret_cast<void *>(CPU_FIRST_ARG(cpu_state)),
        static_cast<int>(CPU_SECOND_ARG(cpu_state)),
        static_cast<int>(CPU_THIRD_ARG(cpu_state)),
        static_cast<int>(CPU_FOURTH_ARG(cpu_state)),
        reinterpret_cast<const char *>(CPU_FIFTH_ARG(cpu_state)),
        reinterpret_cast<const char *>(CPU_SIXTH_ARG(cpu_state)),
        reinterpret_cast<const char *>(CPU_STACK_ARG(cpu_state, 0)),
        reinterpret_cast<const char *>(CPU_STACK_ARG(cpu_state, 1)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 2)),
        reinterpret_cast<const char *>(CPU_STACK_ARG(cpu_state, 3)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 4))));
  }
} // namespace crs
