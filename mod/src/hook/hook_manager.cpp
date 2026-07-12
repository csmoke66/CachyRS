#include "hook.h"

HookManager::HookManager(ProcessInterface* pi)
{
    this->pi = pi;
}

void HookManager::iat(const std::string& name, const std::string &symbol, Hook<void*>* hook)
{
    ImportedFunction fn;
    if (pi->find_import(symbol, &fn))
    {
        hook->trampoline = (void*)*fn.addr;
        iat_hook((void*)fn.addr, hook);

        hooks[name] = hook;
    }
}

void HookManager::x86(const std::string& name, void *target, Hook<void *> *hook)
{
    asm_hook(target, hook);
    hooks[name] = hook;
}