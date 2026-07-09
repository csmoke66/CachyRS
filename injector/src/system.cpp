#include "system.h"
#include <cstring>

uint64_t getlibcaddr(pid_t pid)
{
    char filename[30];
    sprintf(filename, "/proc/%d/maps", pid);

    auto fp = fopen(filename, "r");
    if (!fp)
    {
        return 0;
    }

    char line[850];
    long addr = 0;
    while (!!fgets(line, 850, fp))
    {
        sscanf(line, "%lx-%*lx %*s %*s %*s %*d", &addr);
        if (strstr(line, "libc.so.6") != NULL)
        {
            break;
        }
    }

    fclose(fp);
    return addr;
}

uint64_t get_function_addr(const std::string& name)
{
    void *self = dlopen("libc.so.6", RTLD_LAZY);
    void *addr = dlsym(self, name.c_str());
    return (uint64_t)addr;
}

uint64_t get_function_addr_remote(pid_t pid)
{
    auto our_base = getlibcaddr(getpid());
    auto their_base = getlibcaddr(pid);
    auto our_func = get_function_addr("dlopen");
    return their_base + (our_func - our_base);
}
