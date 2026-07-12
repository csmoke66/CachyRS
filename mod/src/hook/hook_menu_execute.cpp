#include "cachy.h"

void MenuExecuteHook::handler(CpuState *cpu_state)
{
    auto action_menu_context = (ActionMenuContext *)CPU_SECOND_ARG(cpu_state);
    if (auto menu_action_context = action_menu_context->menu_action_context)
    {
        if (auto tmpl = menu_action_context->tmpl)
        {
            LOG(INFO, "Menu execute: " << std::hex << " " << RS.pi.offset(tmpl->handler) << " " << menu_action_context->args[0] << "." << menu_action_context->args[1] << "." << menu_action_context->args[2] << "." << menu_action_context->args[3]);
        }
    }

    cpu_state->rax = (uint64_t)trampoline(
        (void *)CPU_FIRST_ARG(cpu_state),
        action_menu_context,
        (void *)CPU_THIRD_ARG(cpu_state));
}