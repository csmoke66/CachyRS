#pragma once
#include "util.h"

#include <cstdint>
#include <string>

namespace crs
{
    struct ImportedFunction
    {
        ::std::string name;
        uint64_t *addr;
    };
}

#ifdef __linux__
#include "process_linux.h"
#else
UNSUPPORTED_OS();
#endif