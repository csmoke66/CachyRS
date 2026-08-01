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
    void AddChatMessageHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);

        auto name_1 = (JString*)CPU_FIFTH_ARG(cpu_state);
        auto name_2 =  (JString*)CPU_SIXTH_ARG(cpu_state);
        auto name_3 = (JString*)CPU_STACK_ARG(cpu_state, 0);
        auto message = (JString*)CPU_STACK_ARG(cpu_state, 1);
        auto channel = (JString*)CPU_STACK_ARG(cpu_state, 3);
        
        if (!strcmp(message->c_str(), "Welcome to RuneScape."))
        {
            strcpy((char*)strstr(message->c_str(), "RuneScape"), "CachyRS.");
        }

        cpu_state->rax = (uint64_t)trampoline(
            (void *)CPU_FIRST_ARG(cpu_state),
            (int)CPU_SECOND_ARG(cpu_state),
            (int)CPU_THIRD_ARG(cpu_state),
            (int)CPU_FOURTH_ARG(cpu_state),
            (const char*)CPU_FIFTH_ARG(cpu_state),
            (const char*)CPU_SIXTH_ARG(cpu_state),
            (const char*)CPU_STACK_ARG(cpu_state, 0),
            (const char*)CPU_STACK_ARG(cpu_state, 1),
            (void*)CPU_STACK_ARG(cpu_state, 2),
            (const char*)CPU_STACK_ARG(cpu_state, 3),
            (void*)CPU_STACK_ARG(cpu_state, 4));
    }
}
