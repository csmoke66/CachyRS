#include "hook.h"

HookManager::HookManager(ProcessInterface* pi, uint8_t vt_offset)
{
    this->pi = pi;
    this->vt_offset = vt_offset;
}

void HookManager::iat(const std::string& name, const std::string &symbol, std::unique_ptr<Hook<void*>> hook)
{
    ImportedFunction fn;
    if (pi->find_import(symbol, &fn))
    {
        hook->trampoline = (void*)*fn.addr;
        iat_hook(vt_offset, (void*)fn.addr, hook.get());

        hooks[name] = std::move(hook);
    }
}

void HookManager::x86(const std::string& name, void *target, std::unique_ptr<Hook<void *>> hook)
{
    asm_hook(vt_offset, target, hook.get());
    hooks[name] = std::move(hook);
}