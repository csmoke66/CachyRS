#include "tracee.h"
#include <cstddef>
#include <sys/ptrace.h>
#include <cerrno>
#include <cstring>
#include <zero/log.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <elf.h>

Tracee::Tracee(pid_t pid)
{
    this->pid = pid;
}

bool Tracee::attach() const
{
    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0)
    {
        LOG_ERROR("attach process %d failed: %s", pid, strerror(errno));
        return false;
    }

    int s = 0;
    if (waitpid(pid, &s, __WALL | WUNTRACED) != pid)
    {
        LOG_ERROR("wait process %d failed: %s", pid, strerror(errno));
        return false;
    }

    if (WSTOPSIG(s) != SIGSTOP)
    {
        LOG_ERROR("receive signal: %s", strsignal(WSTOPSIG(s)));
        return false;
    }

    return true;
}

bool Tracee::detach() const
{
    if (ptrace(PTRACE_DETACH, pid, nullptr, nullptr) < 0)
    {
        LOG_ERROR("detach process %d failed: %s", pid, strerror(errno));
        return false;
    }

    return true;
}

std::optional<regs_t> Tracee::get_registers() const
{
    regs_t regs = {};

    iovec io = {
        &regs,
        sizeof(regs_t)};

    if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_PRSTATUS, (void *)&io) < 0)
    {
        LOG_ERROR("get process %d registers failed: %s", pid, strerror(errno));
        return std::nullopt;
    }

    return regs;
}

bool Tracee::set_registers(const regs_t &regs) const
{
    iovec io = {
        (void *)&regs,
        sizeof(regs_t)};

    if (ptrace(PTRACE_SETREGSET, pid, (void *)NT_PRSTATUS, (void *)&io) < 0)
    {
        LOG_ERROR("set process %d registers failed: %s", pid, strerror(errno));
        return false;
    }

    return true;
}

std::optional<fp_regs_t> Tracee::get_fp_registers() const
{
    fp_regs_t fp_regs = {};

    iovec io = {
        &fp_regs,
        sizeof(fp_regs_t)};

    if (ptrace(PTRACE_GETREGSET, pid, (void *)NT_FPREGSET, (void *)&io) < 0)
    {
        LOG_ERROR("get process %d fp-registers failed: %s", pid, strerror(errno));
        return std::nullopt;
    }

    return fp_regs;
}

bool Tracee::set_fp_registers(const fp_regs_t &fp_regs) const
{
    iovec io = {
        (void *)&fp_regs,
        sizeof(fp_regs_t)};

    if (ptrace(PTRACE_SETREGSET, pid, (void *)NT_FPREGSET, (void *)&io) < 0)
    {
        LOG_ERROR("set process %d fp-registers failed: %s", pid, strerror(errno));
        return false;
    }

    return true;
}

bool Tracee::read_memory(uintptr_t address, void *buffer, size_t length) const
{
    for (size_t i = 0; i < length; i += sizeof(long))
    {
        long r = ptrace(PTRACE_PEEKTEXT, pid, address + i, nullptr);

        if (r == -1 && errno != 0)
        {
            LOG_ERROR("read process %d memory failed: %s", pid, strerror(errno));
            return false;
        }

        memcpy((char *)buffer + i, &r, std::min(length - i, sizeof(long)));
    }

    return true;
}

bool Tracee::write_memory(uintptr_t address, const void *buffer, size_t length) const
{
    if (length < sizeof(long))
    {
        LOG_ERROR("buffer length need greater than size of long");
        return false;
    }

    for (size_t i = 0; i < length; i += sizeof(long))
    {
        if (length - i < sizeof(long))
        {
            i = length - sizeof(long);
        }

        if (ptrace(PTRACE_POKETEXT, pid, address + i, *(const long *)((const char *)buffer + i)) < 0)
        {
            LOG_ERROR("write process %d memory failed: %s", pid, strerror(errno));
            return false;
        }
    }

    return true;
}

bool Tracee::resume(int sig) const
{
    if (ptrace(PTRACE_CONT, pid, nullptr, sig) < 0)
    {
        LOG_ERROR("resume process %d failed: %s", pid, strerror(errno));
        return false;
    }

    return true;
}

bool Tracee::catch_syscall(int sig) const
{
    if (ptrace(PTRACE_SYSCALL, pid, nullptr, sig) < 0)
    {
        LOG_ERROR("catch process %d syscall failed: %s", pid, strerror(errno));
        return false;
    }

    return true;
}

bool Tracee::set_syscall(long number) const
{
    if (ptrace(PTRACE_POKEUSER, pid, offsetof(regs_t, orig_rax), (void *)number) < 0)
    {
        LOG_ERROR("set process %d syscall failed: %s", pid, strerror(errno));
        return false;
    }

    return true;
}
