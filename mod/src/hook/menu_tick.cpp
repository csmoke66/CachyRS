#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
  void MenuTickHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto menu = reinterpret_cast<Menu *>(CPU_FIRST_ARG(cpu_state));

    auto event = MenuTickEvent(menu);
    RS.event_bus.dispatch(MenuTickEvent::specific_id(), &event);

    trampoline(menu);
  }
} // namespace crs
