#include "plugin_cpp.h"

namespace crs
{
    static PluginApi api;
    static Plugin *plugin = nullptr;
    static ApiEventList<std::function<void()>> tick_events;
    static ApiEventList<std::function<void(MenuActionEventArgs*)>> menu_action_events;
    static std::map<uint64_t, std::shared_ptr<ApiDropDown>> ui_dropdowns;

    static void event_handler_engine_tick(EngineTickArgs *args, void *plugin)
    {
        // clang-format off
        tick_events.iterate([args](auto& f) 
        {
            f();
        });
        // clang-format on
    }

    static void event_handler_menu_action(MenuActionEventArgs *args, void *plugin)
    {
        // clang-format off
        menu_action_events.iterate([args](auto& f) 
        {
            f(args);
        });
        // clang-format on
    }

    void ApiComponent::set_visible(bool visible)
    {
        crs::api.ui_set_visible(this->id, visible);
    }

    void Api::init(crs::InitType type, Plugin *plugin, std::function<void()> first_initializer, std::function<void()> initializer)
    {
        crs::plugin = plugin;
        crs::api = plugin->api;
        if (type == crs::InitType::loaded)
        {
            api.event_bus_register(EngineTickEvent::specific_id().c_str(), (void *)event_handler_engine_tick, nullptr);
            api.event_bus_register(MenuActionEvent::pre_id().c_str(), (void *)event_handler_menu_action, nullptr);
            first_initializer();
        }

        initializer();
    }

    ApiContainer Api::add_container(uint64_t parent_id)
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::container, parent_id);
        return ApiContainer(api, id);
    }

    ApiContainer Api::add_container()
    {
        return add_container(plugin->ui_tab_container_id);
    }

    ApiLabel Api::add_label(uint64_t parent_id, const std::string &text)
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::label, parent_id);
        api.ui_update_component_text(id, text.c_str());
        return ApiLabel(api, id);
    }

    ApiLabel Api::add_label(const std::string &text)
    {
        return add_label(plugin->ui_tab_container_id, text);
    }

    ApiHr Api::add_hr(uint64_t parent_id)
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::hr, parent_id);
        return ApiHr(api, id);
    }

    ApiHr Api::add_hr()
    {
        return add_hr(plugin->ui_tab_container_id);
    }

    ApiCheckBox Api::add_checkbox(uint64_t parent_id, const std::string &text)
    {
        auto id = api.ui_allocate_component(crs::PluginComponentType::checkbox, parent_id);
        api.ui_update_component_text(id, text.c_str());
        return ApiCheckBox(api, id);
    }

    ApiCheckBox Api::add_checkbox(const std::string& text)
    {
        return add_checkbox(plugin->ui_tab_container_id, text);
    }

    static void dropdown_change_handler(uint64_t id, int32_t selected, uint64_t component_id)
    {
        auto component = crs::ui_dropdowns.find(id);
        if (component != crs::ui_dropdowns.end())
        {
            component->second->fire_changed(selected);
        }
    }

    std::shared_ptr<ApiDropDown> Api::add_dropdown(uint64_t parent_id, std::initializer_list<std::string> options)
    {
        std::vector<const char*> converted;
        converted.reserve(options.size());
        for (auto &s : options)
        {
            converted.push_back(s.c_str());
        }

        auto id = api.ui_allocate_component(crs::PluginComponentType::dropdown, parent_id);
        api.ui_update_component_items(id, converted.data(), converted.size());
        api.ui_register_dropdown_change_handler(id, (FnPluginUserInterfaceDropDownChangeHandler)dropdown_change_handler, (void*)id);

        auto dropdown = std::make_shared<ApiDropDown>(api, id);
        crs::ui_dropdowns[id] = dropdown;

        return dropdown;
    }

    std::shared_ptr<ApiDropDown> Api::add_dropdown(std::initializer_list<std::string> options)
    {
        return add_dropdown(plugin->ui_tab_container_id, options);
    }
    
    Globals *Api::raw_globals()
    {
        return api.get_globals().unwrap();
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
        return crs::tick_events.reg(f);
    }

    uint64_t Api::on_menu_action(std::function<void(MenuActionEventArgs*)> f)
    {
        return crs::menu_action_events.reg(f);
    }

    void Api::log(const std::string& s)
    {
        crs::api.log(s.c_str());
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