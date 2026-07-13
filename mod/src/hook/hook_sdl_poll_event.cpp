#include "cachy.h"
#include <imgui.h>

namespace crs
{
    void SdlPollEventHook::handler(CpuState *cpu_state)
    {
        auto event = (SDL_Event *)CPU_FIRST_ARG(cpu_state);
        auto ret = trampoline(event);
        if (ret)
        {
            RS.event_ring_buffer.push(*event);

            auto steal_processing = false;
            auto &io = ImGui::GetIO();
            if (io.WantCaptureMouse || io.WantCaptureKeyboard)
            {
                steal_processing = true;
            }

            if (RS.ui->wants_input())
            {
                steal_processing = true;
            }

            if (steal_processing)
            {
                while (trampoline(event))
                {
                    RS.event_ring_buffer.push(*event);
                }

                cpu_state->rax = 0;
                return;
            }
        }

        cpu_state->rax = ret;
    }
}