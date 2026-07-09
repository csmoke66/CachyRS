#pragma once
#include "tracee.h"
#include <list>
#include <cstdint>

class Executor : public Tracee {
private:
    std::list<int> signals;

public:
    Executor(pid_t pid);
    ~Executor();

public:
    std::optional<int32_t> run(void *shellcode, size_t length, uint64_t base, uint64_t stack, uint64_t argument, uint64_t argument2);
    std::optional<uint64_t> call(void *shellcode, size_t length, uint64_t base, uint64_t stack, uint64_t argument);

private:
    std::optional<uintptr_t> get_executable_memory() const;
};
