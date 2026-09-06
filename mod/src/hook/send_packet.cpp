#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
  void SendPacketHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto container = (PacketContainer *)CPU_SECOND_ARG(cpu_state);

    trampoline(reinterpret_cast<void*>(CPU_FIRST_ARG(cpu_state)),
        container);
  }
} // namespace crs