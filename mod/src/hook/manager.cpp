#include "hook.h"
#include "log.h"

namespace crs
{
  HookManager::HookManager(ProcessInterface *pi, uint8_t vt_offset)
  {
    this->pi = pi;
    this->vt_offset = vt_offset;
  }

  void HookManager::iat(const std::string &name, const std::string &symbol, std::unique_ptr<GenericHook> hook)
  {
    ImportedFunction fn;
    if (pi->find_import(symbol, &fn))
    {
      hook->trampoline = (void *)*fn.addr;
      iat_hook(vt_offset, (void *)fn.addr, hook.get());

      hooks[name] = std::move(hook);
    }
    else
    {
      LOG(ERROR, "Failed to find import with name '" << symbol << "'");
    }
  }

  void HookManager::ptr(const std::string &name, void *address, std::unique_ptr<GenericHook> hook)
  {
    hook->trampoline = *(void **)address;
    iat_hook(vt_offset, address, hook.get());

    hooks[name] = std::move(hook);
  }

  void HookManager::x86(const std::string &name, void *target, std::unique_ptr<GenericHook> hook)
  {
    asm_hook(vt_offset, target, hook.get());
    hooks[name] = std::move(hook);
  }
} // namespace crs