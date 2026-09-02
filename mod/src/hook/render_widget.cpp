#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    void RenderWidgetHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);
        
        auto widget = (Widget *)CPU_FIRST_ARG(cpu_state);
        auto children = (JVector<WidgetChild> *)CPU_THIRD_ARG(cpu_state);
        auto x = (int)CPU_FOURTH_ARG(cpu_state);
        auto y = (int)CPU_FIFTH_ARG(cpu_state);

        auto engine = RS.get_globals()->engine;
        for (auto c = children->begin; c != children->end; c++)
        {
            if (auto w = c->widget)
            {
                // TODO FIXME bad performance
                // *tag = {w, engine->time, x + w->x, y + w->y};
            }
        }

        cpu_state->rax = (uint64_t)trampoline(
            widget,
            (void *)CPU_SECOND_ARG(cpu_state),
            children,
            x,
            y,
            (void *)CPU_SIXTH_ARG(cpu_state),
            (void *)CPU_STACK_ARG(cpu_state, 0),
            (void *)CPU_STACK_ARG(cpu_state, 1),
            (void *)CPU_STACK_ARG(cpu_state, 2),
            (void *)CPU_STACK_ARG(cpu_state, 3),
            (void *)CPU_STACK_ARG(cpu_state, 4),
            (void *)CPU_STACK_ARG(cpu_state, 5),
            (void *)CPU_STACK_ARG(cpu_state, 6));
    }
}