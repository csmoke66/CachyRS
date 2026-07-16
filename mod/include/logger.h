#pragma once
#include <string>

namespace crs
{
    class Logger
    {
    public:
        virtual ~Logger();

    public:
        virtual void log(const ::std::string &s) = 0;

    public:
        void trace(const ::std::string &s);
        void info(const ::std::string &s);
        void warn(const ::std::string &s);
        void error(const ::std::string &s);
    };
}