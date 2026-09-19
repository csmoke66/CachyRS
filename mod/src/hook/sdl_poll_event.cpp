#include "cachy.h"

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

    auto event = reinterpret_cast<SDL_Event *>(CPU_FIRST_ARG(cpu_state));
    auto ret = trampoline(event);
    while (ret)
    {
      if (event->type == SDL_MOUSEMOTION)
      {
        mouse_pos.x = static_cast<float>(event->motion.x);
        mouse_pos.y = static_cast<float>(event->motion.y);
      }

      RS.event_ring_buffer.push(*event);

      auto steal_processing = RS.imgui_want_capture_mouse.load(std::memory_order_relaxed) ||
                              RS.imgui_want_capture_keyboard.load(std::memory_order_relaxed) ||
                              (RS.ui_visible.load(std::memory_order_relaxed) && RS.ui->wants_input()) ||
                              RS.developer_overlay.is_widget_pick_armed();

      if (!wants_event(event->type) || !steal_processing)
      {
        break;
      }

      ret = trampoline(event);
    }

    cpu_state->rax = ret;
  }
} // namespace crs