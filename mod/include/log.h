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
#include <sstream>
#include <iostream>

namespace crs
{
#define LOG(LVL, ...)                                                                                  \
    do                                                                                                 \
    {                                                                                                  \
        std::stringstream ss;                                                                          \
        ss << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << ::std::dec << ::std::endl; \
        std::cout << ss.str();                                                                         \
        std::cout.flush();                                                                             \
    } while (0);
}