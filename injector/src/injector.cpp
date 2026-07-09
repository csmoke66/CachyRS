#include "injector.h"
#include <zero/log.h>
#include <zero/proc/process.h>
#include <dlfcn.h>
#include <stdio.h>
#include <unistd.h>
#include "system.h"

#ifndef PAGE_SIZE
#define PAGE_SIZE 0x1000
#endif

// calls dlopen then performs an exit syscall
// 0:  48 89 fa                mov    rdx,rdi
// 3:  48 89 f7                mov    rdi,rsi
// 6:  48 c7 c6 02 00 00 00    mov    rsi,0x2
// d:  ff d2                   call   rdx
// f:  b8 3c 00 00 00          mov    eax,0x3c
// 14: 0f 05                   syscall
// 16: cc                      int3
unsigned char loader_sc[] = 
{
    0x48, 0x89, 0xFA, 0x48, 0x89, 0xF7, 0x48, 0xC7, 0xC6, 0x02,
    0x00, 0x00, 0x00, 0xFF, 0xD2, 0xB8, 0x3C, 0x00, 0x00, 0x00,
    0x0F, 0x05, 0xCC
};

// 0:  90                      nop
// 1:  90                      nop
// 2:  e8 1b 00 00 00          call   0x22
// 7:  cc                      int3
// 8:  48 89 f8                mov    rax,rdi
// b:  48 89 f7                mov    rdi,rsi
// e:  48 89 d6                mov    rsi,rdx
// 11: 48 89 ca                mov    rdx,rcx
// 14: 4d 89 c2                mov    r10,r8
// 17: 4d 89 c8                mov    r8,r9
// 1a: 4c 8b 4c 24 08          mov    r9,QWORD PTR [rsp+0x8]
// 1f: 0f 05                   syscall
// 21: c3                      ret
// 22: 50                      push   rax
// 23: 45 31 c9                xor    r9d,r9d
// 26: 31 ff                   xor    edi,edi
// 28: 41 b8 ff ff ff ff       mov    r8d,0xffffffff
// 2e: b9 22 00 00 00          mov    ecx,0x22
// 33: ba 07 00 00 00          mov    edx,0x7
// 38: be 00 10 02 00          mov    esi,0x21000
// 3d: e8 31 00 00 00          call   0x73
// 42: 31 d2                   xor    edx,edx
// 44: 48 83 f8 ff             cmp    rax,0xffffffffffffffff
// 48: 48 0f 44 c2             cmove  rax,rdx
// 4c: 5a                      pop    rdx
// 4d: c3                      ret
// 4e: 89 f8                   mov    eax,edi
// 50: 48 c7 c2 ff ff ff ff    mov    rdx,0xffffffffffffffff
// 57: f7 d8                   neg    eax
// 59: 48 81 ff 01 f0 ff ff    cmp    rdi,0xfffffffffffff001
// 60: 48 0f 43 fa             cmovae rdi,rdx
// 64: ba 00 00 00 00          mov    edx,0x0
// 69: 0f 42 c2                cmovb  eax,edx
// 6c: 48 89 c2                mov    rdx,rax
// 6f: 48 89 f8                mov    rax,rdi
// 72: c3                      ret
// 73: 48 83 ec 10             sub    rsp,0x10
// 77: 31 c0                   xor    eax,eax
// 79: 41 51                   push   r9
// 7b: 45 89 c1                mov    r9d,r8d
// 7e: 41 89 c8                mov    r8d,ecx
// 81: 89 d1                   mov    ecx,edx
// 83: 48 89 f2                mov    rdx,rsi
// 86: 48 89 fe                mov    rsi,rdi
// 89: bf 09 00 00 00          mov    edi,0x9
// 8e: e8 75 ff ff ff          call   0x8
// 93: 48 89 c7                mov    rdi,rax
// 96: e8 b3 ff ff ff          call   0x4e
// 9b: 48 83 c4 18             add    rsp,0x18
// 9f: 48 89 d1                mov    rcx,rdx
// a2: 31 d2                   xor    edx,edx
// a4: 48 89 c7                mov    rdi,rax
// a7: 89 d6                   mov    esi,edx
// a9: 48 89 f8                mov    rax,rdi
// ac: bf ff ff ff ff          mov    edi,0xffffffff
// b1: 48 89 f2                mov    rdx,rsi
// b4: 48 c1 e7 20             shl    rdi,0x20
// b8: 89 ce                   mov    esi,ecx
// ba: 48 89 d1                mov    rcx,rdx
// bd: 48 21 f9                and    rcx,rdi
// c0: 48 09 f1                or     rcx,rsi
// c3: 48 89 ca                mov    rdx,rcx
// c6: c3                      ret
unsigned char alloc_sc[] = 
{
    0x90, 0x90, 0xe8, 0x1b, 0x00, 0x00, 0x00, 0xcc, 0x48, 0x89, 0xf8, 0x48,
    0x89, 0xf7, 0x48, 0x89, 0xd6, 0x48, 0x89, 0xca, 0x4d, 0x89, 0xc2, 0x4d,
    0x89, 0xc8, 0x4c, 0x8b, 0x4c, 0x24, 0x08, 0x0f, 0x05, 0xc3, 0x50, 0x45,
    0x31, 0xc9, 0x31, 0xff, 0x41, 0xb8, 0xff, 0xff, 0xff, 0xff, 0xb9, 0x22,
    0x00, 0x00, 0x00, 0xba, 0x07, 0x00, 0x00, 0x00, 0xbe, 0x00, 0x10, 0x02,
    0x00, 0xe8, 0x31, 0x00, 0x00, 0x00, 0x31, 0xd2, 0x48, 0x83, 0xf8, 0xff,
    0x48, 0x0f, 0x44, 0xc2, 0x5a, 0xc3, 0x89, 0xf8, 0x48, 0xc7, 0xc2, 0xff,
    0xff, 0xff, 0xff, 0xf7, 0xd8, 0x48, 0x81, 0xff, 0x01, 0xf0, 0xff, 0xff,
    0x48, 0x0f, 0x43, 0xfa, 0xba, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x42, 0xc2,
    0x48, 0x89, 0xc2, 0x48, 0x89, 0xf8, 0xc3, 0x48, 0x83, 0xec, 0x10, 0x31,
    0xc0, 0x41, 0x51, 0x45, 0x89, 0xc1, 0x41, 0x89, 0xc8, 0x89, 0xd1, 0x48,
    0x89, 0xf2, 0x48, 0x89, 0xfe, 0xbf, 0x09, 0x00, 0x00, 0x00, 0xe8, 0x75,
    0xff, 0xff, 0xff, 0x48, 0x89, 0xc7, 0xe8, 0xb3, 0xff, 0xff, 0xff, 0x48,
    0x83, 0xc4, 0x18, 0x48, 0x89, 0xd1, 0x31, 0xd2, 0x48, 0x89, 0xc7, 0x89,
    0xd6, 0x48, 0x89, 0xf8, 0xbf, 0xff, 0xff, 0xff, 0xff, 0x48, 0x89, 0xf2,
    0x48, 0xc1, 0xe7, 0x20, 0x89, 0xce, 0x48, 0x89, 0xd1, 0x48, 0x21, 0xf9,
    0x48, 0x09, 0xf1, 0x48, 0x89, 0xca, 0xc3
};

// 0:  90                      nop
// 1:  90                      nop
// 2:  e8 1b 00 00 00          call   0x22
// 7:  cc                      int3
// 8:  48 89 f8                mov    rax,rdi
// b:  48 89 f7                mov    rdi,rsi
// e:  48 89 d6                mov    rsi,rdx
// 11: 48 89 ca                mov    rdx,rcx
// 14: 4d 89 c2                mov    r10,r8
// 17: 4d 89 c8                mov    r8,r9
// 1a: 4c 8b 4c 24 08          mov    r9,QWORD PTR [rsp+0x8]
// 1f: 0f 05                   syscall
// 21: c3                      ret
// 22: be 00 10 02 00          mov    esi,0x21000
// 27: e9 25 00 00 00          jmp    0x51
// 2c: 89 f8                   mov    eax,edi
// 2e: 48 c7 c2 ff ff ff ff    mov    rdx,0xffffffffffffffff
// 35: f7 d8                   neg    eax
// 37: 48 81 ff 01 f0 ff ff    cmp    rdi,0xfffffffffffff001
// 3e: 48 0f 43 fa             cmovae rdi,rdx
// 42: ba 00 00 00 00          mov    edx,0x0
// 47: 0f 42 c2                cmovb  eax,edx
// 4a: 48 89 c2                mov    rdx,rax
// 4d: 48 89 f8                mov    rax,rdi
// 50: c3                      ret
// 51: 50                      push   rax
// 52: 48 89 f2                mov    rdx,rsi
// 55: 31 c0                   xor    eax,eax
// 57: 48 89 fe                mov    rsi,rdi
// 5a: bf 0b 00 00 00          mov    edi,0xb
// 5f: e8 a4 ff ff ff          call   0x8
// 64: 48 89 c7                mov    rdi,rax
// 67: e8 c0 ff ff ff          call   0x2c
// 6c: 48 c1 e2 20             shl    rdx,0x20
// 70: 89 c0                   mov    eax,eax
// 72: 48 09 d0                or     rax,rdx
// 75: 5a                      pop    rdx
// 76: c3                      ret
unsigned char free_sc[] = 
{
    0x90, 0x90, 0xe8, 0x1b, 0x00, 0x00, 0x00, 0xcc, 0x48, 0x89, 0xf8, 0x48,
    0x89, 0xf7, 0x48, 0x89, 0xd6, 0x48, 0x89, 0xca, 0x4d, 0x89, 0xc2, 0x4d,
    0x89, 0xc8, 0x4c, 0x8b, 0x4c, 0x24, 0x08, 0x0f, 0x05, 0xc3, 0xbe, 0x00,
    0x10, 0x02, 0x00, 0xe9, 0x25, 0x00, 0x00, 0x00, 0x89, 0xf8, 0x48, 0xc7,
    0xc2, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xd8, 0x48, 0x81, 0xff, 0x01, 0xf0,
    0xff, 0xff, 0x48, 0x0f, 0x43, 0xfa, 0xba, 0x00, 0x00, 0x00, 0x00, 0x0f,
    0x42, 0xc2, 0x48, 0x89, 0xc2, 0x48, 0x89, 0xf8, 0xc3, 0x50, 0x48, 0x89,
    0xf2, 0x31, 0xc0, 0x48, 0x89, 0xfe, 0xbf, 0x0b, 0x00, 0x00, 0x00, 0xe8,
    0xa4, 0xff, 0xff, 0xff, 0x48, 0x89, 0xc7, 0xe8, 0xc0, 0xff, 0xff, 0xff,
    0x48, 0xc1, 0xe2, 0x20, 0x89, 0xc0, 0x48, 0x09, 0xd0, 0x5a, 0xc3
};

// The amount of memory allocated by the allocate shellcode
constexpr auto ShellcodeAllocSize = 0x21000;

Injector::~Injector()
{
    for (const auto &executor : executors)
    {
        std::ignore = executor->detach();
        delete executor;
    }
}

bool Injector::open(pid_t pid)
{
    this->pid = pid;

    auto threads = zero::proc::getThreads(pid);
    if (!threads)
    {
        LOG_ERROR("get process %d threads failed", pid);
        return false;
    }

    for (const auto &tid : *threads)
    {
        std::unique_ptr<Executor> executor(new Executor(tid));
        if (!executor->attach())
        {
            return false;
        }

        executors.push_back(executor.release());
    }

    LOG_INFO("attach process %d success", pid);
    return true;
}

int32_t Injector::inject(const std::string &library)
{
    LOG_INFO("execute alloc shellcode");

    auto executor = executors.front();
    auto result = executor->call(alloc_sc, sizeof(alloc_sc), 0, 0, 0);
    if (!result || !*result)
    {
        LOG_ERROR("execute alloc shellcode failed");
        return -1;
    }

    auto name_result = executor->call(alloc_sc, sizeof(alloc_sc), 0, 0, 0);
    if (!name_result || !*name_result)
    {
        LOG_ERROR("execute alloc name failed");
        return -1;
    }

    LOG_INFO("memory allocated: %p", result);
    std::optional<regs_t> regs = executor->get_registers();
    std::optional<fp_regs_t> fp_regs = executor->get_fp_registers();
    if (!regs || !fp_regs)
    {
        LOG_ERROR("get executor context failed");
        return -1;
    }

    if (!executor->write_memory(*name_result, library.data(), library.size() + 1))
    {
        LOG_ERROR("write name failed");
        return -1;
    }

    uint64_t base = *result;
    uint64_t stack = *result + ShellcodeAllocSize;

    LOG_INFO("execute loader shellcode");
    auto status = executor->run(loader_sc, sizeof(loader_sc), base, stack, get_function_addr_remote(pid), *name_result);
    if (!status)
    {
        LOG_ERROR("execute loader shellcode failed");
        return -1;
    }

    LOG_INFO("execute free shellcode");
    if (!executor->call(free_sc, sizeof(free_sc), 0, 0, *result))
    {
        LOG_ERROR("execute free shellcode failed");
        return -1;
    }

    return *status;
}
