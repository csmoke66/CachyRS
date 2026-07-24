#include "log.h"

namespace crs
{
    void Log::init(const std::string &file)
    {
        log_stream = std::ofstream(file);
    }

    Log logx;
}