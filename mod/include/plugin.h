#pragma once
#include <vector>
#include <memory>

#include "reversed/reversed.h"

#include "ownership.h"

#define PLUGIN_API __attribute((visibility("default"))) extern "C"

namespace crs
{
    struct Plugin;
    typedef void (*FnPluginInit)(Plugin* plugin);

    typedef void (*FnPluginLog)(const char* message);

    typedef ThreadOwned<Globals*> (*FnPluginGetGlobals)();

    typedef void (*FnPluginEventBusReceiver)(void* args, void* context);
    typedef void (*FnPluginEventBusRegister)(const char* id, void* receiver, void* context);

    struct PluginApi
    {
        FnPluginLog log;
        FnPluginGetGlobals get_globals;

        FnPluginEventBusRegister event_bus_register;
    };

    struct Plugin
    {
        const char* name = nullptr;
        PluginApi api;
    };

    class PluginManager
    {
    private:
        ::std::vector<::std::unique_ptr<Plugin>> plugins;
        PluginApi api;

    public:
        void init();

    private:
        void load(const ::std::string& path);

    public:
        void load_all(const ::std::string& path);
    };
}