#pragma once
#include <fstream>
#include <mutex>

#define LOG(LVL, ...)                                                                                      \
    logx.log_mutex.lock();                                                                                  \
    logx.log_stream << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
    logx.log_mutex.unlock();                                                                                \
    logx.flush();

class Log
{
public:
    std::ofstream log_stream;
    std::mutex log_mutex;

public:
    void init(const std::string &file);
    void flush();
};

// If you rename this to log, it will compile, the application will do nothing
// and you will be in compiler/linker bug hell. You can name it log, but only
// if no STL objects are in the log object. Why? Because fuck you that's why.
// Hours of my life I'll never get back.
extern Log logx;