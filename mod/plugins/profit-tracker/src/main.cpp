#include "plugin.h"
#include "event_bus.h"
#include "game_events.h"
#include <format>
#include <sstream>

class ProfitTrackerPlugin
{
public:
    crs::Plugin *plugin;
    crs::PluginApi *api;

public:
    uint64_t ui_profit;
    uint64_t profit;
};

static ProfitTrackerPlugin profit_tracker_plugin;

static std::map<int32_t, int32_t> amounts = {
    {49450, 3017},  /* gold coin */
    {49510, 2617},  /* imperial steel*/
    {49506, 2589},  /* ancient vis */
    {49508, 5993},  /* blood of orkus */
    {49458, 3145},  /* soap stone */
    {49512, 6068},  /* tyrian purple */
    {49514, 47481}, /* zaorisan signal */
    {49456, 2590},  /* samite silk */
};

static void event_handler_item_changed(crs::ItemChangedArgs *args, ProfitTrackerPlugin *plugin)
{
    if (args->id == 93)
    {
        if (args->stack_delta > 0)
        {
            auto it = amounts.find(args->new_id);
            if (it != amounts.end())
            {
                plugin->profit += (args->stack_delta * it->second);
            }
        }

        std::stringstream ss;
        ss << plugin->profit;

        plugin->api->ui_update_component_text(plugin->ui_profit, ss.str().c_str());
    }
}

PLUGIN_API
const char *plugin_get_name()
{
    return "Profit Tracker";
}

PLUGIN_API
void plugin_init(crs::InitType type, crs::Plugin *plugin)
{
    if (type == crs::InitType::loaded)
    {
        profit_tracker_plugin.plugin = plugin;
        profit_tracker_plugin.api = &plugin->api;

        auto &api = plugin->api;
        api.event_bus_register("on_set_item_container", (void *)&event_handler_item_changed, &profit_tracker_plugin);
    }

    auto &api = plugin->api;

    profit_tracker_plugin.ui_profit = api.ui_allocate_component(crs::PluginComponentType::label, plugin->ui_tab_container_id);
}