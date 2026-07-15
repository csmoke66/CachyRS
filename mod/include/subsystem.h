#pragma once
#include <string>
#include <map>
#include <memory>
#include <concepts>

namespace crs
{
    class SubSystem
    {
    public:
        virtual ::std::string get_id() = 0;
    };

    class SubSystemCache
    {
    private:
        ::std::map<std::string, ::std::shared_ptr<SubSystem>> subsystems;

    private:
        ::std::shared_ptr<SubSystem> get_subsystem_generic(const ::std::string& id);

    public:
        template <std::derived_from<SubSystem> T>
        ::std::shared_ptr<T> get_subsystem(const std::string& id)
        {
            return (::std::shared_ptr<T>)get_subsystem_generic(id);
        }
    };
}