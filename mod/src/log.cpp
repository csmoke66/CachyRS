#include "log.h"

void Log::init(const std::string& file)
{
    log_stream = std::ofstream(file);
}

void Log::flush()
{
    log_mutex.lock();
    log_stream.flush();
    log_mutex.unlock();
}

Log logx;