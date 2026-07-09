#include "executor.h"
#include <zero/log.h>
#include <zero/proc/process.h>
#include <algorithm>
#include <sys/wait.h>
#include <syscall.h>

#define REG_PC rip
#define REG_ARG rdi
#define REG_ARG2 rsi
#define REG_RET rax
#define REG_STACK rsp
#define PC_OFFSET 2
#define REG_SYSCALL orig_rax
#define REG_SYSCALL_ARG rdi
#define REG_SYSCALL_ARG1 rsi
#define REG_SYSCALL_ARG2 rdx

constexpr auto PRIVATE_EXIT_MAGIC = 0x6861636b;

constexpr auto PRIVATE_EXIT_SYSCALL = SYS_sched_yield;
constexpr auto PRIVATE_CANCEL_SYSCALL = -1;

Executor::Executor(pid_t pid) : Tracee(pid)
{
}

Executor::~Executor()
{
    for (const auto &sig : signals)
    {
        kill(pid, sig);
    }
}

std::optional<int32_t> Executor::run(void *shellcode, size_t length, uint64_t base, uint64_t stack, uint64_t argument, uint64_t argument2)
{
    base = base ? base : get_executable_memory().value_or(0);
    if (!base)
    {
        LOG_ERROR("get executable memory base failed");
        return std::nullopt;
    }

    LOG_INFO("write shellcode: %p[0x%lx]", base, length);

    auto buffer = std::make_unique<char[]>(length);
    if (!read_memory(base, buffer.get(), length))
    {
        return std::nullopt;
    }

    if (!write_memory(base, shellcode, length))
    {
        return std::nullopt;
    }

    auto regs = get_registers();
    auto fp_regs = get_fp_registers();
    if (!regs || !fp_regs)
    {
        return std::nullopt;
    }

    LOG_INFO("entry: %p stack: %p argument: 0x%lx", base, stack, argument);

    regs_t modify = *regs;

    modify.REG_PC = base + PC_OFFSET;
    modify.REG_STACK = stack ? stack : modify.REG_STACK;

    modify.REG_ARG = argument;
    modify.REG_ARG2 = argument2;

    if (!set_registers(modify))
    {
        return std::nullopt;
    }

    int32_t sig = 0;
    int32_t status = 0;
    bool exiting = false;
    while (true)
    {
        if (!catch_syscall(sig))
        {
            return std::nullopt;
        }

        int32_t s = 0;
        if (waitpid(pid, &s, __WALL) < 0)
        {
            LOG_ERROR("wait pid failed: %s", strerror(errno));
            return std::nullopt;
        }

        if (WIFSIGNALED(s))
        {
            LOG_WARNING("process terminated: %s", strsignal(WTERMSIG(s)));
            return std::nullopt;
        }

        auto current = get_registers();

        if (!current)
        {
            return std::nullopt;
        }

        sig = WSTOPSIG(s);
        if (sig == SIGSEGV)
        {
            LOG_ERROR("segmentation fault: %p", current->REG_PC);
            break;
        }

        if (sig != SIGTRAP)
        {
            LOG_INFO("receive signal: %s", strsignal(sig));
            continue;
        }

        sig = 0;
        if (exiting && current->REG_SYSCALL == PRIVATE_CANCEL_SYSCALL)
        {
            LOG_INFO("catch exit syscall: %d", status);
            break;
        }

        if ((int32_t)current->REG_SYSCALL == PRIVATE_EXIT_SYSCALL &&
            current->REG_SYSCALL_ARG1 == PRIVATE_EXIT_MAGIC)
        {
            status = (int32_t)current->REG_SYSCALL_ARG2;
            if (!set_syscall(PRIVATE_CANCEL_SYSCALL))
            {
                return std::nullopt;
            }

            exiting = true;
            continue;
        }

        if (current->REG_SYSCALL == SYS_exit || current->REG_SYSCALL == SYS_exit_group)
        {
            status = (int32_t)current->REG_SYSCALL_ARG;
            if (!set_syscall(PRIVATE_CANCEL_SYSCALL))
            {
                return std::nullopt;
            }

            exiting = true;
        }
    }

    if (!write_memory(base, buffer.get(), length))
    {
        return std::nullopt;
    }

    if (!set_registers(*regs) || !set_fp_registers(*fp_regs))
    {
        return std::nullopt;
    }

    if (sig == SIGSEGV)
    {
        return std::nullopt;
    }

    return status;
}

std::optional<uint64_t> Executor::call(void *shellcode, size_t length, uint64_t base, uint64_t stack, uint64_t argument)
{
    base = base ? base : get_executable_memory().value_or(0);
    if (!base)
    {
        LOG_ERROR("get executable memory base failed");
        return std::nullopt;
    }

    LOG_INFO("write shellcode: %p[0x%lx]", base, length);
    auto buffer = std::make_unique<char[]>(length);

    if (!read_memory(base, buffer.get(), length))
    {
        return std::nullopt;
    }

    if (!write_memory(base, shellcode, length))
    {
        return std::nullopt;
    }

    auto regs = get_registers();
    auto fp_regs = get_fp_registers();

    if (!regs || !fp_regs)
    {
        return std::nullopt;
    }

    LOG_INFO("entry: %p stack: %p argument: 0x%lx", base, stack, argument);

    auto modify = *regs;
    modify.REG_PC = base + PC_OFFSET;
    modify.REG_STACK = stack ? stack : modify.REG_STACK;

    modify.REG_ARG = argument;

    if (!set_registers(modify))
    {
        return std::nullopt;
    }

    int32_t sig = 0;
    uint64_t result = 0;
    while (true)
    {
        if (!resume(sig))
        {
            return std::nullopt;
        }

        int32_t s = 0;
        if (waitpid(pid, &s, __WALL) < 0)
        {
            LOG_ERROR("wait pid failed: %s", strerror(errno));
            return std::nullopt;
        }

        if (WIFSIGNALED(s))
        {
            LOG_WARNING("process terminated: %s", strsignal(WTERMSIG(s)));
            return std::nullopt;
        }

        auto current = get_registers();
        if (!current)
        {
            return std::nullopt;
        }

        sig = WSTOPSIG(s);
        if (sig == SIGSEGV)
        {
            LOG_ERROR("segmentation fault: %p", current->REG_PC);
            break;
        }

        if (sig == SIGTRAP)
        {
            result = current->REG_RET;
            break;
        }

        LOG_INFO("receive signal: %s", strsignal(sig));
    }

    if (!write_memory(base, buffer.get(), length))
    {
        return std::nullopt;
    }

    if (!set_registers(*regs) || !set_fp_registers(*fp_regs))
    {
        return std::nullopt;
    }

    if (sig == SIGSEGV)
    {
        return std::nullopt;
    }

    return result;
}

std::optional<uintptr_t> Executor::get_executable_memory() const
{
    auto process_mappings = zero::proc::getProcessMappings(pid);
    if (!process_mappings)
    {
        LOG_ERROR("get process %d memory mappings failed", pid);
        return std::nullopt;
    }

    auto it = std::find_if(
        process_mappings->begin(),
        process_mappings->end(),
        [](const auto &m)
        {
            return (m.permissions & zero::proc::READ_PERMISSION) &&
                   (m.permissions & zero::proc::EXECUTE_PERMISSION) &&
                   (m.permissions & zero::proc::PRIVATE_PERMISSION);
        });

    if (it == process_mappings->end())
    {
        return std::nullopt;
    }

    return it->start;
}
