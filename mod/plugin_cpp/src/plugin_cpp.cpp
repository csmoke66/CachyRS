#include "plugin_cpp.h"
#include <game_events.h>

namespace crs
{
    PluginApi Api::api;
    Plugin *Api::plugin = nullptr;
    ApiEventList<std::function<void()>> Api::tick_events;

    static void event_handler_engine_tick(EngineTickArgs *args, void *plugin)
    {
        // clang-format off
        Api::tick_events.iterate([args](auto& f) 
        {
            f();
        });
        // clang-format on
    }

    void Api::init(crs::InitType type, Plugin *plugin, std::function<void()> first_initializer, std::function<void()> initializer)
    {
        Api::plugin = plugin;
        Api::api = plugin->api;
        if (type == crs::InitType::loaded)
        {
            api.event_bus_register(EngineTickEvent::specific_id().c_str(), (void *)event_handler_engine_tick, nullptr);
            first_initializer();
        }

        initializer();
    }

    ApiLabel Api::add_label(const std::string &text)
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::label, plugin->ui_tab_container_id);
        api.ui_update_component_text(id, text.c_str());
        return ApiLabel(api, id);
    }

    ApiHr Api::add_hr()
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::hr, plugin->ui_tab_container_id);
        return ApiHr(api, id);
    }

    ApiCheckBox Api::add_checkbox(const std::string &text)
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::checkbox, plugin->ui_tab_container_id);
        api.ui_update_component_text(id, text.c_str());
        return ApiCheckBox(api, id);
    }

    Engine *Api::raw_engine()
    {
        return api.get_globals()->engine;
    }

    PlayerUpdateCache *Api::raw_player_update_cache()
    {
        auto engine = Api::raw_engine();
        if (!engine)
        {
            return nullptr;
        }

        return engine->player_update_cache;
    }

    NpcUpdateCache *Api::raw_npc_update_cache()
    {
        auto engine = Api::raw_engine();
        if (!engine)
        {
            return nullptr;
        }

        return engine->npc_update_cache;
    }

    Player *Api::raw_self()
    {
        auto engine = Api::raw_engine();
        if (!engine)
        {
            return nullptr;
        }

        auto lp = engine->local_player;
        if (!lp)
        {
            return nullptr;
        }

        auto player_update_cache = Api::raw_player_update_cache();
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

        return update->player;
    }

    std::vector<Player *> Api::raw_players()
    {
        std::vector<Player *> players;
        if (auto cache = raw_player_update_cache())
        {
            for (auto it = cache->updates.begin; it != cache->updates.end; it++)
            {
                if (auto update = *it)
                {
                    if (auto player = update->player)
                    {
                        players.push_back(player);
                    }
                }
            }
        }

        return players;
    }

    std::vector<Npc *> Api::raw_npcs()
    {
        std::vector<Npc *> npcs;
        if (auto cache = raw_npc_update_cache())
        {
            for (auto i = 0; i < cache->size; i++)
            {
                if (auto update = cache->npcs[i])
                {
                    if (auto npc = update->npc)
                    {
                        npcs.push_back(npc);
                    }
                }
            }
        }

        return npcs;
    }

    crs::SocialCache *Api::raw_social_cache()
    {
        auto engine = Api::raw_engine();
        if (!engine)
        {
            return nullptr;
        }

        return engine->social_cache;
    }

    bool Api::raw_is_friend(const crs::Player *player)
    {
        auto cache = Api::raw_social_cache();
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

    ApiPlayer Api::self()
    {
        return ApiPlayer(Api::raw_self());
    }

    std::vector<ApiPlayer> Api::players(std::function<bool(ApiPlayer &)> conditional)
    {
        std::vector<ApiPlayer> players;
        for (auto player : Api::raw_players())
        {
            auto api = ApiPlayer(player);
            if (conditional(api))
            {
                players.push_back(api);
            }
        }

        return players;
    }

    std::vector<ApiNpc> Api::npcs(std::function<bool(ApiNpc &)> conditional)
    {
        std::vector<ApiNpc> npcs;
        for (auto npc : Api::raw_npcs())
        {
            auto api = ApiNpc(npc);
            if (conditional(api))
            {
                npcs.push_back(api);
            }
        }

        return npcs;
    }

    uint64_t Api::on_tick(std::function<void()> f)
    {
        return Api::tick_events.reg(f);
    }
}

PLUGIN_API
const char *plugin_get_name()
{
    static char plugin_name_cached[256];
    strcpy(plugin_name_cached, crs::Boot::name().c_str());
    return plugin_name_cached;
}

PLUGIN_API
void plugin_init(crs::InitType type, crs::Plugin *plugin)
{
    // clang-format off
    crs::Api::init(type, plugin, []()
    {
        crs::Boot::init();
    }, []()
    {
        crs::Boot::init_ui();
    });
}