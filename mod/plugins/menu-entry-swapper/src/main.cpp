#include "plugin.h"
#include "event_bus.h"
#include "game_events.h"
#include <format>

class MenuEntrySwapperPlugin
{
public:
    crs::Plugin *plugin;
    crs::PluginApi *api;

public:
    uint64_t ui_pickpocket;

public:
    bool pickpocket_enabled()
    {
        return api->ui_is_component_checked(ui_pickpocket);
    }
};

static MenuEntrySwapperPlugin menu_entry_swapper_plugin;

static void event_handler_engine_tick(void *args, MenuEntrySwapperPlugin *plugin)
{
    if (auto engine = plugin->api->get_globals()->engine)
    {
        if (auto menu = engine->menu)
        {
            auto &opt = menu->menu_options;
            auto n = opt.size();
            if (n > 1)
            {
                for (int i = 0; i < n - 1; i++)
                {
                    for (int j = 0; j < n - i - 1; j++)
                    {
                        auto a = opt.reference(j)->body;
                        auto b = opt.reference(j + 1)->body;

                        if (a->action_template->type < b->action_template->type)
                        {
                            auto tmp = *a;
                            *a = *b;
                            *b = tmp;
                        }
                    }
                }
            }
        }
    }
}

PLUGIN_API
const char *plugin_get_name()
{
    return "Menu Entry Swapper";
}

PLUGIN_API
void plugin_init(crs::InitType type, crs::Plugin *plugin)
{
    if (type == crs::InitType::loaded)
    {
        menu_entry_swapper_plugin.plugin = plugin;
        menu_entry_swapper_plugin.api = &plugin->api;

        auto &api = plugin->api;
        api.event_bus_register("on_engine_tick", (void *)&event_handler_engine_tick, &menu_entry_swapper_plugin);
    }

    auto &api = plugin->api;

    menu_entry_swapper_plugin.ui_pickpocket = api.ui_allocate_component(crs::PluginComponentType::checkbox, plugin->ui_tab_container_id);
    api.ui_update_component_text(menu_entry_swapper_plugin.ui_pickpocket, "Pickpocket");
}