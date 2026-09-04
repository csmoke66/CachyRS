#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
  void RenderWidgetHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto widget = (Widget *)CPU_FIRST_ARG(cpu_state);
    auto children = (JVector<WidgetChild> *)CPU_THIRD_ARG(cpu_state);
    auto x = static_cast<int>(CPU_FOURTH_ARG(cpu_state));
    auto y = static_cast<int>(CPU_FIFTH_ARG(cpu_state));

    auto engine = RS.get_globals()->engine;
    auto time = engine->time;

    for (auto c = children->begin; c != children->end; c++)
    {
      if (auto w = c->widget)
      {
        auto it = snapshots.find(w);
        if (it == snapshots.end())
        {
          auto n = RenderedWidgetSnapshot();
          n.widget = w;

          snapshots[w] = n;
          it = snapshots.find(w);
        }

        it->second.time = time;
      }
    }

    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
        widget,
        reinterpret_cast<void *>(CPU_SECOND_ARG(cpu_state)),
        children,
        x,
        y,
        reinterpret_cast<void *>(CPU_SIXTH_ARG(cpu_state)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 0)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 1)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 2)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 3)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 4)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 5)),
        reinterpret_cast<void *>(CPU_STACK_ARG(cpu_state, 6))));
  }

  bool RenderWidgetHook::is_visible(const Widget *w) const
  {
    return snapshots.find(w) != snapshots.end();
  }

  void RenderWidgetHook::remove_stale(uint32_t before)
  {
    for (auto it = snapshots.begin(); it != snapshots.end();)
    {
      if (it->second.time < before)
      {
        it = snapshots.erase(it);
      }
      else
      {
        it++;
      }
    }
  }
} // namespace crs