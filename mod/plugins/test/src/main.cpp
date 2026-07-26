#include "plugin.h"
#include "event_bus.h"
#include <format>

static void event_handler_engine_tick(void* args, crs::Plugin* plugin)
{
    auto& api = plugin->api;
    api.log(std::format("wow {}", plugin->name).c_str());
}

PLUGIN_API
void plugin_init(crs::Plugin *plugin)
{
    plugin->name = "Test";
    
    auto& api = plugin->api;
    api.event_bus_register("on_engine_tick", (void*)&event_handler_engine_tick, plugin);
}