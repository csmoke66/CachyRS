#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    void SetVarBitHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);

        auto wsc = (const WorldSettingCache*)CPU_FIRST_ARG(cpu_state);
        auto buffer = (const CacheBuffer<void, WorldSettingMask>*)CPU_SECOND_ARG(cpu_state);
        auto value = (const uint32_t*)CPU_THIRD_ARG(cpu_state);

        LOG(INFO, "Update world setting: " << buffer->body->world_setting_id << " to " << *value);
        cpu_state->rax = (uint64_t)trampoline(wsc, buffer, value);
    }
}