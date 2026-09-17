#include "hook.h"
#include "log.h"

namespace crs
{
  void BaseHook::handler(CpuState *)
  {
    auto tid = std::this_thread::get_id();
    if (last_thread_id.has_value() && tid != last_thread_id)
    {
      LOG(WARN, "Hook called on multiple threads (Initial: " << last_thread_id.value() << ", Current: " << tid << ")");
    }

    last_thread_id = tid;
    call_count += 1;
  }

  std::optional<std::thread::id> BaseHook::thread_id() const
  {
    return last_thread_id;
  }
} // namespace crs