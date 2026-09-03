#include "cachy.h"

namespace crs
{
  void MenuExecuteHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto action_menu_context = (ActionMenuContext *)CPU_SECOND_ARG(cpu_state);
    auto menu_action_context = action_menu_context->menu_action_context;

    auto pre_event = MenuActionEvent(MenuActionEvent::pre_id(), &menu_action_context->args, &menu_action_context->tmpl);
    RS.event_bus.dispatch(MenuActionEvent::pre_id(), &pre_event);

    auto tmpl = menu_action_context->tmpl;
    LOG(INFO, "Menu execute: " << std::hex << RS.pi.offset((void *)tmpl->handler)
                               << std::dec
                               << " Type: " << static_cast<uint32_t>(tmpl->type)
                               << " Id:" << tmpl->id
                               << " Args:"
                               << menu_action_context->args.r[0] << "."
                               << menu_action_context->args.r[1] << "."
                               << menu_action_context->args.r[2] << "."
                               << menu_action_context->args.r[3]);

    if (pre_event.args.bypass_logic)
    {
      cpu_state->rax = reinterpret_cast<uint64_t>(tmpl->handler(tmpl, action_menu_context));
    }
    else
    {
      cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
          reinterpret_cast<void *>(CPU_FIRST_ARG(cpu_state)),
          action_menu_context,
          reinterpret_cast<void *>(CPU_THIRD_ARG(cpu_state))));
    }

    auto post_event = MenuActionEvent(MenuActionEvent::post_id(), &menu_action_context->args, &menu_action_context->tmpl);
    RS.event_bus.dispatch(MenuActionEvent::post_id(), &post_event);
  }
} // namespace crs