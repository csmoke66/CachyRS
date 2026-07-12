#include "util.h"
#include <unistd.h>
#include <sys/mman.h>

#include "cachy.h"

void *allocate_executable_memory(void *data, size_t size)
{
    auto page_size = sysconf(_SC_PAGESIZE);
    auto mem = mmap(NULL, page_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mem == MAP_FAILED)
    {
        LOG(ERROR, "Failed to allocate memory");
        return nullptr;
    }

    memcpy(mem, data, size);
    if (mprotect(mem, page_size, PROT_READ | PROT_EXEC) == -1)
    {
        LOG(ERROR, "Failed make memory executable");
        munmap(mem, page_size);
        return nullptr;
    }

    return mem;
}

void patch_non_writable_memory(void *target, void *data, size_t size, int prot)
{
    auto page = (Elf64_Addr)target & ~(getpagesize() - 1);

    mprotect((void *)page, getpagesize(), prot | PROT_WRITE);
    memcpy(target, data, size);
    mprotect((void *)page, getpagesize(), prot);
}