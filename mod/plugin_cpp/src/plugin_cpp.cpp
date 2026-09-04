#include "plugin_cpp.h"

namespace crs
{
  static PluginApi api;
  static Plugin *plugin = nullptr;
  static ApiEventList<std::function<void()>> tick_events;
  static ApiEventList<std::function<void(MenuActionEventArgs *)>> menu_action_events;
  static ApiEventList<std::function<void(uint32_t, uint32_t)>> world_setting_changed_events;
  static std::map<uint64_t, std::shared_ptr<ApiDropDown>> ui_dropdowns;

  static MenuActionTemplate menu_action_override_template;
  static bool has_menu_action_override = false;
  static FnMenuActionHandler menu_action_override_handler;
  static MenuActionArgs menu_action_override_args;
  static bool menu_action_override_bypass;

  static void event_handler_engine_tick(EngineTickArgs *args, void *)
  {
    tick_events.iterate([args](auto &f)
    {
      f();
    });
  }

  static void event_handler_menu_action(MenuActionEventArgs *args, void *)
  {
    menu_action_events.iterate([args](auto &f)
    {
      f(args);
    });

    if (has_menu_action_override)
    {
      memcpy(&menu_action_override_template, *args->action_template, sizeof(MenuActionTemplate));
      menu_action_override_template.handler = menu_action_override_handler;
      *args->action_template = &menu_action_override_template;
      *args->args = menu_action_override_args;
      args->bypass_logic = menu_action_override_bypass;

      has_menu_action_override = false;
    }
  }

  static void event_handler_world_setting_changed(WorldSettingChangedEventArgs *args, void *)
  {
    world_setting_changed_events.iterate([args](auto &f)
    {
      f(args->world_setting_id, args->value);
    });
  }

  uint64_t ApiComponent::get_id()
  {
    return this->id;
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
      api.event_bus_register(WorldSettingChangedEvent::specific_id().c_str(), (void *)event_handler_world_setting_changed, nullptr);
      first_initializer();
    }

    initializer();
  }

  uint64_t Api::root_plugin_component_id()
  {
    return plugin->ui_tab_container_id;
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

  ApiCheckBox Api::add_checkbox(const std::string &text)
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

  std::shared_ptr<ApiDropDown> Api::add_dropdown(uint64_t parent_id, const std::vector<std::string> &options)
  {
    std::vector<const char *> converted;
    converted.reserve(options.size());
    for (auto &s : options)
    {
      converted.push_back(s.c_str());
    }

    auto id = api.ui_allocate_component(crs::PluginComponentType::dropdown, parent_id);
    api.ui_update_component_items(id, converted.data(), converted.size());

    api.ui_register_dropdown_change_handler(id,
        reinterpret_cast<FnPluginUserInterfaceDropDownChangeHandler>(dropdown_change_handler),
        reinterpret_cast<void *>(id));

    auto dropdown = std::make_shared<ApiDropDown>(api, id);
    crs::ui_dropdowns[id] = dropdown;

    return dropdown;
  }

  std::shared_ptr<ApiDropDown> Api::add_dropdown(const std::vector<std::string> &options)
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

  WorldSettingCache *Api::raw_world_setting_cache()
  {
    auto engine = Api::raw_engine();
    if (!engine)
    {
      return nullptr;
    }

    return &engine->world_settings;
  }

  WidgetCache *Api::raw_widget_cache()
  {
    auto engine = Api::raw_engine();
    if (!engine)
    {
      return nullptr;
    }

    return engine->widget_cache;
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

  uint32_t Api::get_world_setting(uint32_t id)
  {
    auto cache = Api::raw_world_setting_cache();
    if (!cache)
    {
      Api::log("raw_world_setting_cache is NULL");
      return 0;
    }

    auto chunk = id % cache->count;
    auto c = cache->vars[chunk];
    while (c)
    {
      if (c->id == id)
      {
        return c->body.value;
      }

      c = c->body.next;
    }

    Api::log(std::format("Failed to find world setting with id '{}'", id));
    return 0;
  }

  std::optional<ApiItemContainer> Api::get_item_container(uint32_t id, uint16_t parent_widget, uint16_t child_widget)
  {
    auto engine = Api::raw_engine();
    if (!engine)
    {
      return std::optional<ApiItemContainer>();
    }

    auto item_cache = engine->item_cache;
    if (!item_cache)
    {
      return std::optional<ApiItemContainer>();
    }

    for (auto i = item_cache->containers.begin; i != item_cache->containers.end; i++)
    {
      if (i->id == id)
      {
        std::vector<ApiItem> items;

        auto slot = 0;
        for (auto item = i->items.begin; item != i->items.end; item++)
        {
          if (item->id != -1)
          {
            items.push_back(ApiItem(parent_widget, child_widget, slot, item->id, item->amount));
          }

          slot += 1;
        }

        return ApiItemContainer(i->items.size(), items);
      }
    }

    return std::optional<ApiItemContainer>();
  }

  std::optional<ApiItemContainer> Api::get_inventory()
  {
    return Api::get_item_container(93, 1473, 5);
  }

  bool Api::has_selected_item()
  {
    auto cache = Api::raw_widget_cache();
    if (!cache)
    {
      return false;
    }

    return cache->widget_item_selected != 0;
  }

  FnMenuActionHandler Api::get_menu_action_handler(MenuActionType type, uint32_t idx)
  {
    static uint64_t obj_offsets[] = {
      off(Globals, menu_action_handler_obj1),
      off(Globals, menu_action_handler_obj2),
      off(Globals, menu_action_handler_obj3),
      off(Globals, menu_action_handler_obj4),
      off(Globals, menu_action_handler_obj5),
      off(Globals, menu_action_handler_obj6),
      off(Globals, menu_action_handler_obj7)
    };

    static uint64_t npc_offsets[] = {
      off(Globals, menu_action_handler_npc1),
      off(Globals, menu_action_handler_npc2),
      off(Globals, menu_action_handler_npc3),
      off(Globals, menu_action_handler_npc4),
      off(Globals, menu_action_handler_npc5),
      off(Globals, menu_action_handler_npc6),
      off(Globals, menu_action_handler_npc7)
    };

    static uint64_t widget_offsets[] = {
      off(Globals, menu_action_handler_widget1),
      off(Globals, menu_action_handler_widget2),
      off(Globals, menu_action_handler_widget3)
    };

    auto globals = Api::raw_globals();
    if (type == MenuActionType::walk)
    {
      return reinterpret_cast<FnMenuActionHandler>(&globals->menu_action_handler_walk);
    }
    else if (type == MenuActionType::obj)
    {
      return reinterpret_cast<FnMenuActionHandler>(reinterpret_cast<char *>(globals) + obj_offsets[idx]);
    }
    else if (type == MenuActionType::npc)
    {
      return reinterpret_cast<FnMenuActionHandler>(reinterpret_cast<char *>(globals) + npc_offsets[idx]);
    }
    else if (type == MenuActionType::widget)
    {
      return reinterpret_cast<FnMenuActionHandler>(reinterpret_cast<char *>(globals) + widget_offsets[idx]);
    }

    return nullptr;
  }

  void Api::perform_menu_action(FnMenuActionHandler handler, const MenuActionArgs &args)
  {
    MenuActionTemplate templ;
    templ.engine = Api::raw_engine();
    templ.handler = handler;

    MenuActionContext ctx;
    ctx.tmpl = &templ;
    ctx.args = args;

    ActionMenuContext am_ctx;
    am_ctx.menu_action_context = &ctx;

    Api::log(std::format("test {} {} {} {}", ctx.args.r[0], ctx.args.r[1], ctx.args.r[2], ctx.args.r[3]));
    handler(&templ, &am_ctx);
  }

  void Api::select_item(uint16_t parent_widget, uint16_t child_widget, int32_t slot)
  {
    MenuActionArgs args;
    args.args_widget.option_idx = 0;
    args.args_widget.sub_idx = slot;
    args.args_widget.widget_id = ((uint32_t)parent_widget << 16) | child_widget;
    args.args_widget.always_1 = 1;

    perform_menu_action(Api::get_menu_action_handler(MenuActionType::widget, 1), args);
  }

  void Api::override_current_menu_action(FnMenuActionHandler handler, const MenuActionArgs &args, bool bypass)
  {
    crs::has_menu_action_override = true;
    crs::menu_action_override_handler = handler;
    crs::menu_action_override_args = args;
    crs::menu_action_override_bypass = bypass;
  }

  uint64_t Api::on_tick(std::function<void()> f)
  {
    return crs::tick_events.reg(f);
  }

  uint64_t Api::on_menu_action(std::function<void(MenuActionEventArgs *)> f)
  {
    return crs::menu_action_events.reg(f);
  }

  uint64_t Api::on_world_setting_changed(std::function<void(uint32_t, uint32_t)> f)
  {
    return crs::world_setting_changed_events.reg(f);
  }

  void Api::log(const std::string &s)
  {
    crs::api.log(s.c_str());
  }
} // namespace crs

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
  crs::Api::init(type, plugin, []()
  {
    crs::Boot::init();
  }, []()
  {
    crs::Boot::init_ui();
  });
}