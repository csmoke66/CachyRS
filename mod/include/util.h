#pragma once
#include <cstdint>
#include <cstddef>
#include <initializer_list>

#define FINLINE __attribute__((always_inline)) inline
#define UNSUPPORTED_OS() static_assert(false, "Unsupported operating system")
#define off(t, f) offsetof(t, f)

void* allocate_executable_memory(void* data, size_t size);
void patch_non_writable_memory(void* target, void* data, size_t size, int prot);

template <typename T, typename... A>
FINLINE T dref(const void *obj, std::initializer_list<uint64_t> args)
{
    const void *last = obj;
    for (auto a : args)
    {
        last = *((const void **)((const char *)last + a));
    }

    return (T)last;
}
