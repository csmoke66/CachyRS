#include "plugin.h"
#include "event_bus.h"
#include <format>

class EntityHiderPlugin
{
public:
    crs::Plugin *plugin;
    crs::PluginApi *api;

public:
    uint64_t ui_players_hide_checkbox;
    uint64_t ui_players_show_self_checkbox;
    uint64_t ui_players_show_friends_checkbox;

public:
    bool players_hidden()
    {
        return api->ui_is_component_checked(ui_players_hide_checkbox);
    }

    bool players_show_self()
    {
        return api->ui_is_component_checked(ui_players_show_self_checkbox);
    }

    bool players_show_friends()
    {
        return api->ui_is_component_checked(ui_players_show_friends_checkbox);
    }

public:
    const crs::SocialCache *social_cache() const
    {
        auto globals = api->get_globals();
        auto engine = globals->engine;
        if (!engine)
        {
            return nullptr;
        }

        return engine->social_cache;
    }

    bool is_friend(const crs::Player *player) const
    {
        auto cache = social_cache();
        if (!cache)
        {
            return false;
        }

        for (auto i = cache->friends.begin; i != cache->friends.end; i++)
        {
            if (!strcmp(player->name.c_str(), i->name.c_str()))
            {
                return true;
            }
        }

        return false;
    }

    crs::Player *local_player_entity() const
    {
        auto globals = api->get_globals();
        auto engine = globals->engine;
        if (!engine)
        {
            return nullptr;
        }

        auto lp = engine->local_player;
        if (!lp)
        {
            return nullptr;
        }

        auto player_update_cache = engine->player_update_cache;
        if (!player_update_cache)
        {
            return nullptr;
        }

        auto idx = lp->entity_list_index;
        if (idx < 0 || idx >= player_update_cache->updates.size())
        {
            return nullptr;
        }

        auto update = *player_update_cache->updates.reference(idx);
        if (!update)
        {
            return nullptr;
        }

        return (crs::Player *)update->entity;
    }

    template <typename FN>
    void iterate_entities(crs::WorldNode *node, FN func)
    {
        if (!node)
        {
            return;
        }

        if (node->entity)
        {
            func(node, node->entity);
        }

        for (auto c = node->children.begin; c != node->children.end; c++)
        {
            iterate_entities(*c, func);
        }
    }

    template <typename FN>
    void iterate_entities(FN func)
    {
        auto globals = api->get_globals();
        auto engine = globals->engine;
        if (!engine)
        {
            return;
        }

        auto scene = engine->scene_001;
        if (!scene)
        {
            return;
        }

        auto scene_002 = scene->scene_002.reference(scene->scene_index);
        if (!scene_002)
        {
            return;
        }

        auto scene_003 = scene_002->scene_003;
        if (!scene_003)
        {
            return;
        }

        iterate_entities(scene_003->world_root, func);
    }
};

static EntityHiderPlugin entity_hider_plugin;

static void event_handler_engine_tick(void *args, EntityHiderPlugin *plugin)
{
    auto api = plugin->api;
    auto lp = plugin->local_player_entity();

    // clang-format off
    plugin->iterate_entities([plugin, lp](crs::WorldNode* world_node, const crs::Entity *entity) 
    {
        if (entity->type == crs::EntityType::player)
        {
            auto hidden = plugin->players_hidden();
            if (hidden)
            {
                if (plugin->players_show_self())
                {
                    if (entity == lp)
                    {
                        hidden = false;
                    }
                }
                
                if (plugin->players_show_friends())
                {
                    if (plugin->is_friend((crs::Player*)entity))
                    {
                        hidden = false;
                    }
                }
            }

            if (hidden)
            {
                world_node->flags &= ~crs::WorldNodeFlag::has_entity;
            }
            else
            {
                world_node->flags |= crs::WorldNodeFlag::has_entity;
            }
        }
    });
    // clang-format on
}

PLUGIN_API
const char* plugin_get_name()
{
    return "Entity Hider";
}

PLUGIN_API
void plugin_init(crs::InitType type, crs::Plugin *plugin)
{
    if (type == crs::InitType::loaded)
    {
        entity_hider_plugin.plugin = plugin;
        entity_hider_plugin.api = &plugin->api;

        auto &api = plugin->api;
        api.event_bus_register("on_engine_tick", (void *)&event_handler_engine_tick, &entity_hider_plugin);
    }

    auto &api = plugin->api;

    auto label = api.ui_allocate_component(crs::PluginComponentType::label, plugin->ui_tab_container_id);
    api.ui_update_component_text(label, "Players");
    api.ui_allocate_component(crs::PluginComponentType::hr, plugin->ui_tab_container_id);

    entity_hider_plugin.ui_players_hide_checkbox = api.ui_allocate_component(crs::PluginComponentType::checkbox, plugin->ui_tab_container_id);
    api.ui_update_component_text(entity_hider_plugin.ui_players_hide_checkbox, "Hidden");

    entity_hider_plugin.ui_players_show_self_checkbox = api.ui_allocate_component(crs::PluginComponentType::checkbox, plugin->ui_tab_container_id);
    api.ui_update_component_text(entity_hider_plugin.ui_players_show_self_checkbox, "Show Self");

    entity_hider_plugin.ui_players_show_friends_checkbox = api.ui_allocate_component(crs::PluginComponentType::checkbox, plugin->ui_tab_container_id);
    api.ui_update_component_text(entity_hider_plugin.ui_players_show_friends_checkbox, "Show Friends");
}