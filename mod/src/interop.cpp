#include "interop.h"
#include "util.h"

#include <cstdlib>

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#else
UNSUPPORTED_OS();
#endif

namespace crs
{
#ifdef __linux__
    static std::string interop_linux_get_home_directory()
    {
        auto pwd = getpwuid(getuid());
        if (pwd && pwd->pw_dir)
        {
            return std::string(pwd->pw_dir);
        }

        return "";
    }
#else
    UNSUPPORTED_OS();
#endif

    std::string interop_get_home_directory()
    {
#ifdef __linux__
        return interop_linux_get_home_directory();
#else
        UNSUPPORTED_OS();
#endif
    }
}