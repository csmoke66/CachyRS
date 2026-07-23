#include "hook.h"
#include "log.h"

namespace crs
{
    void BaseHook::handler(CpuState *cpu_state)
    {
        auto tid = std::this_thread::get_id();
        if (this->last_thread_id.has_value() && tid != this->last_thread_id)
        {
            LOG(WARN, "Hook called on multiple threads (Initial: " << 
                this->last_thread_id.value() << ", Current: " << tid << ")");
        }

        this->last_thread_id = tid;
    }

    std::thread::id BaseHook::thread_id() const
    {
        return this->last_thread_id.value();
    }
}