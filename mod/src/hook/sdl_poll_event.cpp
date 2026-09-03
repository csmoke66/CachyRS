#include "cachy.h"
#include <imgui.h>

namespace crs
{
  void SdlPollEventHook::handler(CpuState *cpu_state)
  {
    auto wants_event = [](uint32_t type)
    {
      return type == SDL_MOUSEMOTION ||
             type == SDL_MOUSEBUTTONDOWN ||
             type == SDL_MOUSEBUTTONUP ||
             type == SDL_MOUSEWHEEL ||
             type == SDL_KEYDOWN ||
             type == SDL_KEYUP;
    };

    BaseHook::handler(cpu_state);

    auto event = (SDL_Event *)CPU_FIRST_ARG(cpu_state);
    auto ret = trampoline(event);
    while (ret)
    {
      RS.event_ring_buffer.push(*event);

      auto &io = ImGui::GetIO();
      auto steal_processing = io.WantCaptureMouse || io.WantCaptureKeyboard || (RS.ui_visible && RS.ui->wants_input());

      if (!wants_event(event->type) || !steal_processing)
      {
        break;
      }

      ret = trampoline(event);
    }

    cpu_state->rax = ret;
  }
} // namespace crs