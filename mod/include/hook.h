#pragma once
#include <cstdint>
#include <map>
#include <memory>

#include "process.h"
#include "reversed.h"

#pragma pack(push, 1)
struct Xmm
{
    union
    {
        uint64_t qword[2];
    };
};

#ifdef __linux__
#define CPU_FIRST_ARG(C) C->rdi
#define CPU_SECOND_ARG(C) C->rsi
#define CPU_THIRD_ARG(C) C->rdx
#define CPU_FOURTH_ARG(C) C->rcx
#else
UNSUPPORTED_OS();
#endif

struct CpuState
{
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t rsp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    Xmm xmm[16];
};
#pragma pack(pop)

template <typename T>
class Hook
{
public:
    T trampoline;

public:
    virtual ~Hook();

public:
    virtual void handler(CpuState *cpu_state) = 0;
};

template<typename T>
Hook<T>::~Hook()
{

}

class DummyHook : public Hook<void*>
{
public:
    void handler(CpuState *cpu_state);
};

class MenuExecuteHook : public Hook<FnMenuExecute>
{
public:
    void handler(CpuState *cpu_state) override;
};

class EglSwapBuffersHook : public Hook<FnEglSwapBuffers>
{
private:
    EGLint cached_width, cached_height;
    bool is_first_run = true;

public:
    void handler(CpuState *cpu_state) override;
};

class SdlPollEventHook : public Hook<FnSDL_PollEvent>
{
public:
    void handler(CpuState *cpu_state) override;
};

class HookManager
{
private:
    ProcessInterface *pi;
    uint8_t vt_offset;
    std::map<std::string, std::unique_ptr<Hook<void *>>> hooks;

public:
    HookManager(ProcessInterface *pi, uint8_t vt_offset);

public:
    void iat(const std::string& name, const std::string &symbol, std::unique_ptr<Hook<void *>> hook);
    void x86(const std::string& name, void *target, std::unique_ptr<Hook<void *>> hook);
};

template<typename T>
FINLINE std::unique_ptr<Hook<void*>> unique_hook()
{
    return std::unique_ptr<Hook<void*>>((Hook<void*>*)new T());
}

void asm_init();
void asm_hook(uint8_t vt_offset, void *target, Hook<void *> *hook);
void iat_hook(uint8_t vt_offset, void *target, Hook<void *> *hook);