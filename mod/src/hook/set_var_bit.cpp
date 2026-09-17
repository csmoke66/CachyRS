#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
  void SetVarBitHook::handler(CpuState *cpu_state)
  {
    BaseHook::handler(cpu_state);

    auto wsc = reinterpret_cast<const WorldSettingCache *>(CPU_FIRST_ARG(cpu_state));
    auto buffer = reinterpret_cast<const CacheBuffer<void, WorldSettingMask> *>(CPU_SECOND_ARG(cpu_state));
    auto value = reinterpret_cast<const uint32_t *>(CPU_THIRD_ARG(cpu_state));

    auto event = WorldSettingChangedEvent(buffer->body->world_setting_id, *value);
    RS.event_bus.dispatch(WorldSettingChangedEvent::specific_id(), &event);

    void *tmp[8];
    auto fn = reinterpret_cast<void (*)(void *, void *, void *, uint32_t)>(reinterpret_cast<char *>(RS.get_globals().unwrap()) + 0xb9b9e0);
    uint32_t id = buffer->body->world_setting_id;
    fn(tmp, reinterpret_cast<char *>(const_cast<WorldSettingCache *>(wsc)) + 0x20, &id, id);

    LOG(INFO, "Update world setting: " << buffer->body->world_setting_id << " to " << *value << " at " << tmp[0]);
    cpu_state->rax = reinterpret_cast<uint64_t>(trampoline(wsc, buffer, value));
  }
} // namespace crs
