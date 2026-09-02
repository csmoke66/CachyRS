#pragma once
#include <vector>
#include <memory>
#include <functional>

#include "reversed/reversed.h"

#include "ownership.h"

#define PLUGIN_API __attribute((visibility("default"))) extern "C"

namespace crs
{
    enum class PluginComponentType
    {
        container,
        label,
        hr,
        line,
        button,
        checkbox,
        dropdown,
    };

    enum class InitType
    {
        loaded,
        refreshed
    };

    struct Plugin;
    
    typedef const char* (*FnPluginGetName)();
    typedef void (*FnPluginInit)(InitType type, Plugin *plugin);
    typedef void (*FnPluginLog)(const char *message);
    typedef void (*FnPluginUserInterfaceDropDownChangeHandler)(uint64_t id, int32_t selected, void* user_data);

    typedef ThreadOwned<Globals *> (*FnPluginGetGlobals)();

    typedef uint64_t (*FnPluginUserInterfaceAllocateComponent)(PluginComponentType type, uint64_t parent_id);
    typedef void (*FnPluginUserInterfaceUpdateComponentText)(uint64_t component_id, const char* text);
    typedef void (*FnPluginUserInterfaceUpdateComponentItems)(uint64_t component_id, const char** items, size_t item_count);
    typedef bool (*FnPluginUserInterfaceIsComponentChecked)(uint64_t component_id);
    typedef void (*FnPluginUserInterfaceRegisterDropDownChangeHandler)(uint64_t component_id, FnPluginUserInterfaceDropDownChangeHandler handler, void* user_data);
    typedef void (*FnPluginUserInterfaceSetVisible)(uint64_t component_id, bool visible);
    
    typedef void (*FnPluginEventBusReceiver)(void* args, void* context);
    typedef void (*FnPluginEventBusRegister)(const char* id, void* receiver, void* context);

    struct PluginApi
    {
        FnPluginLog log;
        FnPluginGetGlobals get_globals;

        FnPluginUserInterfaceAllocateComponent ui_allocate_component;
        FnPluginUserInterfaceUpdateComponentText ui_update_component_text;
        FnPluginUserInterfaceUpdateComponentItems ui_update_component_items;
        FnPluginUserInterfaceIsComponentChecked ui_is_component_checked;
        FnPluginUserInterfaceRegisterDropDownChangeHandler ui_register_dropdown_change_handler;
        FnPluginUserInterfaceSetVisible ui_set_visible;

        FnPluginEventBusRegister event_bus_register;
    };

    struct Plugin
    {
        const char *name = nullptr;
        uint64_t ui_tab_container_id;

        FnPluginGetName get_name;
        FnPluginInit init;

        PluginApi api;
    };

    class PluginManager
    {
    private:
        ::std::vector<::std::unique_ptr<Plugin>> plugins;
        PluginApi api;

    private:
        std::vector<std::function<void(Plugin*)>> plugin_load_callbacks;

    public:
        void init();
        void add_load_callback(std::function<void(Plugin*)> function);

    private:
        void load(const ::std::string &path);

    public:
        void load_all(const ::std::string &path);
        
    public:
        const std::vector<std::unique_ptr<Plugin>>& view_plugins() const;
    };
}