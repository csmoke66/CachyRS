#include "plugin.h"
#include "event_bus.h"
#include <format>

struct TestPlugin
{
    crs::Plugin *plugin;
    crs::PluginApi *api;

    std::uint32_t last_animation;
};

static void event_handler_engine_tick(void *args, TestPlugin *plugin)
{
    auto api = plugin->api;

    auto globals = api->get_globals();
    auto engine = globals->engine;
    if (!engine)
    {
        return;
    }

    auto lp = engine->local_player;
    if (!lp)
    {
        return;
    }

    auto player_update_cache = engine->player_update_cache;
    if (!player_update_cache)
    {
        return;
    }

    auto idx = lp->entity_list_index;
    if (idx < 0 || idx >= player_update_cache->updates.size())
    {
        return;
    }

    auto update = *player_update_cache->updates.reference(idx);
    if (!update)
    {
        return;
    }

    auto entity = update->entity;
    if (!entity)
    {
        return;
    }
    
    auto &anim_queue = entity->animation_queue;
    if (anim_queue.size() == 0)
    {
        if (plugin->last_animation != 0)
        {
            api->log(std::format("Animation stopped from {}", plugin->last_animation).c_str());
            plugin->last_animation = 0;
        }
        return;
    }

    auto anim = *anim_queue.reference(0);
    if (anim != plugin->last_animation)
    {
        api->log(std::format("Animation changed from {} to {}", plugin->last_animation, anim).c_str());
        plugin->last_animation = anim;
    }
}

PLUGIN_API
void plugin_init(crs::Plugin *plugin)
{
    plugin->name = "Test";

    auto tp = new TestPlugin();
    tp->plugin = plugin;
    tp->api = &plugin->api;

    auto &api = plugin->api;
    api.event_bus_register("on_engine_tick", (void *)&event_handler_engine_tick, tp);
}