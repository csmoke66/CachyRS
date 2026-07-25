#pragma once
//
// I'm going to put a small note here about design decisions.
//
// Globals will be restricted to the internal client, and will not be exposed
// as globals to public facing APIs. I do not think it's worth the maintenance
// cost to start passing dependencies like log objects into every aspect of the
// application. There are obvious "downsides" to this, such as not being
// able to unit test (why would I unit test writing to an ofstream?) and not being
// able to dynamically swap the logger.
//
// The core of CachyRS is extremely easy to reason about because dependencies are
// not vertically stacked, and I plan to keep it that way. Abstractions are used
// where they have real benefits, such as the UserInterface backends.
//
#include <fstream>
#include <mutex>

namespace crs
{
#define LOG(LVL, ...)                                                                                           \
    logx.log_mutex.lock();                                                                                      \
    logx.log_stream << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << ::std::dec << ::std::endl; \
    logx.log_stream.flush();                                                                                               \
    logx.log_mutex.unlock();

    class Log
    {
    public:
        ::std::ofstream log_stream;
        ::std::mutex log_mutex;

    public:
        void init(const ::std::string &file);
    };

    // If you rename this to log, it will compile, the application will do nothing
    // and you will be in compiler/linker bug hell. You can name it log, but only
    // if no STL objects are in the log object. Why? Because fuck you that's why.
    // Hours of my life I'll never get back.
    extern Log logx;
}