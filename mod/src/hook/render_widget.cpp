#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
  void RenderWidgetHook::handler(CpuState *cpu_state)
  {
    auto children = reinterpret_cast<JVector<WidgetChild> *>(CPU_THIRD_ARG(cpu_state));
    auto x = static_cast<int>(CPU_FOURTH_ARG(cpu_state));
    auto y = static_cast<int>(CPU_FIFTH_ARG(cpu_state));

    auto *globals = reinterpret_cast<Globals *>(RS.pi.game_base());
    auto time = globals->engine->time;

    for (auto c = children->begin(); c != children->end(); c++)
    {
      if (auto w = c->widget)
      {
        auto &snap = snapshots[w];
        snap.widget = w;
        snap.parent = w->parent;
        snap.absolute_x = x + static_cast<int32_t>(w->x);
        snap.absolute_y = y + static_cast<int32_t>(w->y);
        snap.width = w->width;
        snap.height = w->height;
        snap.time = time;
        snap.has_menu_options = !w->menu_options.empty();
      }
    }

    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(
        reinterpret_cast<Widget *>(CPU_FIRST_ARG(cpu_state)),
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

  const std::unordered_map<const Widget *, RenderedWidgetSnapshot> &RenderWidgetHook::rendered() const
  {
    return snapshots;
  }

  bool RenderWidgetHook::is_visible(const Widget *w) const
  {
    auto it = snapshots.find(w);
    if (it == snapshots.end())
    {
      return false;
    }

    return it->second.width > 0 && it->second.height > 0;
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
        ++it;
      }
    }
  }
} // namespace crs
