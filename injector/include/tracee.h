#pragma once

#include <sys/user.h>
#include <sys/types.h>
#include <cstdint>
#include <optional>

typedef user_regs_struct regs_t;
typedef user_fpregs_struct fp_regs_t;

class Tracee
{
protected:
    pid_t pid;

public:
    Tracee(pid_t pid);

public:
    bool attach() const;
    bool detach() const;

public:
    bool resume(int sig) const;
    bool catch_syscall(int sig) const;
    bool set_syscall(long number) const;

public:
    std::optional<regs_t> get_registers() const;
    bool set_registers(const regs_t &regs) const;

    std::optional<fp_regs_t> get_fp_registers() const;
    bool set_fp_registers(const fp_regs_t &fp_regs) const;

    bool read_memory(uintptr_t address, void *buffer, size_t length) const;
    bool write_memory(uintptr_t address, const void *buffer, size_t length) const;
};
