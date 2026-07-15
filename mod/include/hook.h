#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <concepts>

#include "process.h"
#include "reversed/reversed.h"

namespace crs
{
    class RenderWidgetHook;
    class SdlPollEventHook;
    
#pragma pack(push, 1)
    struct Xmm
    {
        union
        {
            uint64_t qword[2];
        };
    };

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

#ifdef __linux__
#define CPU_FIRST_ARG(C) C->rdi
#define CPU_SECOND_ARG(C) C->rsi
#define CPU_THIRD_ARG(C) C->rdx
#define CPU_FOURTH_ARG(C) C->rcx
#define CPU_FIFTH_ARG(C) C->r8
#define CPU_SIXTH_ARG(C) C->r9
// + 0x18 because the stack is offset by 0x18 to make room for XMM registers
#define CPU_STACK_ARG(C, I) *((uint64_t *)(C->rsp + 0x18 + sizeof(uint64_t) + (sizeof(uint64_t) * I)))
#else
    UNSUPPORTED_OS();
#endif
#pragma pack(pop)

    class BaseHook
    {
    public:
        virtual ~BaseHook()
        {

        }

    public:
        virtual void handler(CpuState *cpu_state) = 0;
    };

    template <typename T>
    class Hook : public BaseHook
    {
    public:
        T trampoline;
    };

    typedef Hook<void*> GenericHook;

    class DummyHook : public BaseHook
    {
    public:
        void handler(CpuState *cpu_state);
    };

    class HookManager
    {
    private:
        ProcessInterface *pi;
        uint8_t vt_offset;
        ::std::map<std::string, std::unique_ptr<BaseHook>> hooks;

    public:
        HookManager(ProcessInterface *pi, uint8_t vt_offset);

    public:
        template <std::derived_from<BaseHook> T>
        const T *view_hook(const ::std::string &name)
        {
            return (const T *)hooks[name].get();
        }

    public:
        void iat(const ::std::string &name, const ::std::string &symbol, ::std::unique_ptr<GenericHook> hook);
        void x86(const ::std::string &name, void *target, ::std::unique_ptr<GenericHook> hook);
    };

    template <std::derived_from<BaseHook> T>
    FINLINE ::std::unique_ptr<GenericHook> unique_hook()
    {
        return ::std::unique_ptr<GenericHook>((GenericHook *)new T());
    }

    void asm_init();
    void asm_hook(uint8_t vt_offset, void *target, GenericHook *hook);
    void iat_hook(uint8_t vt_offset, void *target, GenericHook *hook);
}