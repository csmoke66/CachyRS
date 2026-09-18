#include "plugin_cpp.h"
#include "plugin_cpp_detail.h"

#include "version.hpp"

#include <atomic>
#include <cstring>
#include <iterator>
#include <mutex>

namespace crs
{
  static PluginHostApi api;

  static Plugin *plugin = nullptr;
  static bool forced_on = false;
  static ApiCheckBox enabled_checkbox;
  static std::atomic<bool> enabled_flag{ false };
  static ApiContainer plugin_content;
  static bool plugin_content_visible = true;

  static ApiEventList<std::function<void()>> tick_events;
  static ApiEventList<std::function<void(MenuActionEventArgs *)>> menu_action_events;
  static ApiEventList<std::function<void(bool)>> menu_opened_events;
  static ApiEventList<std::function<void(uint32_t, uint32_t)>> world_setting_changed_events;
  static ApiEventList<std::function<void(uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t)>> item_changed_events;
  static ApiEventList<std::function<void(const std::string &, const std::string &, const std::string &)>> new_chat_message_events;

  static std::mutex ui_component_maps_mu;
  static std::map<uint64_t, std::shared_ptr<ApiDropDown>> ui_dropdowns;
  static std::map<uint64_t, std::shared_ptr<ApiButton>> ui_buttons;
  static std::map<uint64_t, std::shared_ptr<ApiGraphMap>> ui_graph_maps;

  static MenuActionTemplate menu_action_override_template;
  static bool has_menu_action_override = false;
  static FnMenuActionHandler menu_action_override_handler;
  static MenuActionArgs menu_action_override_args;
  static bool menu_action_override_bypass;

  static bool is_enabled()
  {
    return forced_on || enabled_flag.load(std::memory_order_relaxed);
  }

  static void sync_plugin_content_visibility()
  {
    if (plugin_content.get_id() == static_cast<uint64_t>(-1))
    {
      return;
    }

    const bool want = is_enabled();
    if (want == plugin_content_visible)
    {
      return;
    }

    plugin_content_visible = want;
    plugin_content.set_visible(want);
  }

  static uint64_t default_ui_parent()
  {
    if (plugin_content.get_id() != static_cast<uint64_t>(-1))
    {
      return plugin_content.get_id();
    }
    return plugin->ui_tab_container_id;
  }

  bool Api::enabled()
  {
    return is_enabled();
  }

  static void event_handler_engine_tick(EngineTickArgs *, void *)
  {
    const bool overlay_open = api.ui_is_visible && api.ui_is_visible();
    if (!forced_on && overlay_open)
    {
      enabled_flag.store(enabled_checkbox.is_checked(), std::memory_order_relaxed);
    }
    sync_plugin_content_visibility();

    if (is_enabled())
    {
      tick_events.iterate([](auto &fn)
      {
        fn();
      });
    }
  }

  static void event_handler_menu_opened(MenuOpenedEventArgs *args, void *)
  {
    if (is_enabled())
    {
      menu_opened_events.iterate([args](auto &fn)
      {
        fn(args->opened);
      });
    }
  }

  static void event_handler_menu_action(MenuActionEventArgs *args, void *)
  {
    if (is_enabled())
    {
      menu_action_events.iterate([args](auto &fn)
      {
        fn(args);
      });

      if (has_menu_action_override)
      {
        if (auto *current = *args->action_template)
        {
          memcpy(&menu_action_override_template, current, sizeof(MenuActionTemplate));
        }
        else
        {
          menu_action_override_template = MenuActionTemplate{};
          menu_action_override_template.engine = Api::raw_engine();
        }

        menu_action_override_template.handler = menu_action_override_handler;
        *args->action_template = &menu_action_override_template;
        *args->args = menu_action_override_args;
        args->bypass_logic = menu_action_override_bypass;

        has_menu_action_override = false;
      }
    }
  }

  static void event_handler_world_setting_changed(WorldSettingChangedEventArgs *args, void *)
  {
    if (is_enabled())
    {
      world_setting_changed_events.iterate([args](auto &fn)
      {
        fn(args->world_setting_id, args->value);
      });
    }
  }

  static void event_handler_item_changed(ItemChangedArgs *args, void *)
  {
    if (is_enabled())
    {
      item_changed_events.iterate([args](auto &fn)
      {
        fn(
            args->id,
            args->slot,
            args->old_id,
            args->old_amount,
            args->new_id,
            args->new_amount,
            args->stack_delta);
      });
    }
  }

  static void event_handler_new_chat_message(NewChatMessageArgs *args, void *)
  {
    if (is_enabled())
    {
      new_chat_message_events.iterate([args](auto &fn)
      {
        fn(args->channel, args->sender, args->message);
      });
    }
  }

  uint64_t ApiComponent::get_id() const
  {
    return id;
  }

  void ApiComponent::set_visible(bool visible)
  {
    crs::api.ui_set_visible(id, visible);
  }

  template <typename T>
  static T resolve_fn(const PluginApi &table, const char *name)
  {
    if (!table.resolve)
    {
      return nullptr;
    }
    return reinterpret_cast<T>(table.resolve(name));
  }

  static PluginHostApi bind_host_api(const PluginApi &table)
  {
    PluginHostApi host{};
    host.log = resolve_fn<FnPluginLog>(table, "log");
    host.get_configuration_dir = resolve_fn<FnPluginGetConfigurationDir>(table, "get_configuration_dir");
    host.get_globals = resolve_fn<FnPluginGetGlobals>(table, "get_globals");

    host.ui_allocate_component = resolve_fn<FnPluginUserInterfaceAllocateComponent>(table, "ui_allocate_component");
    host.ui_update_component_text = resolve_fn<FnPluginUserInterfaceUpdateComponentText>(table, "ui_update_component_text");
    host.ui_update_component_items = resolve_fn<FnPluginUserInterfaceUpdateComponentItems>(table, "ui_update_component_items");
    host.ui_is_component_active = resolve_fn<FnPluginUserInterfaceIsComponentActive>(table, "ui_is_component_active");
    host.ui_set_component_active = resolve_fn<FnPluginUserInterfaceSetActive>(table, "ui_set_component_active");
    host.ui_register_dropdown_change_handler = resolve_fn<FnPluginUserInterfaceRegisterDropDownChangeHandler>(table, "ui_register_dropdown_change_handler");
    host.ui_register_button_click_handler = resolve_fn<FnPluginUserInterfaceRegisterButtonClickHandler>(table, "ui_register_button_click_handler");
    host.ui_dropdown_set_selected = resolve_fn<FnPluginUserInterfaceDropDownSetSelected>(table, "ui_dropdown_set_selected");
    host.ui_set_visible = resolve_fn<FnPluginUserInterfaceSetVisible>(table, "ui_set_visible");
    host.ui_update_graph_map = resolve_fn<FnPluginUserInterfaceUpdateGraphMap>(table, "ui_update_graph_map");
    host.ui_update_graph_map_primitives = resolve_fn<FnPluginUserInterfaceUpdateGraphMapPrimitives>(table, "ui_update_graph_map_primitives");
    host.ui_register_graph_map_select_handler = resolve_fn<FnPluginUserInterfaceRegisterGraphMapSelectHandler>(table, "ui_register_graph_map_select_handler");
    host.ui_register_graph_map_link_handler = resolve_fn<FnPluginUserInterfaceRegisterGraphMapLinkHandler>(table, "ui_register_graph_map_link_handler");
    host.ui_register_graph_map_object_link_handler = resolve_fn<FnPluginUserInterfaceRegisterGraphMapObjectLinkHandler>(table, "ui_register_graph_map_object_link_handler");
    host.ui_register_graph_map_background_handler = resolve_fn<FnPluginUserInterfaceRegisterGraphMapBackgroundHandler>(table, "ui_register_graph_map_background_handler");
    host.ui_register_graph_map_context_handler = resolve_fn<FnPluginUserInterfaceRegisterGraphMapContextHandler>(table, "ui_register_graph_map_context_handler");
    host.ui_register_graph_map_zoom_handler = resolve_fn<FnPluginUserInterfaceRegisterGraphMapZoomHandler>(table, "ui_register_graph_map_zoom_handler");

    host.event_bus_register = resolve_fn<FnPluginEventBusRegister>(table, "event_bus_register");
    host.expose_function = resolve_fn<FnPluginExposeFunction>(table, "expose_function");
    host.get_exposed_function = resolve_fn<FnPluginGetExposedFunction>(table, "get_exposed_function");
    host.ui_is_visible = resolve_fn<FnPluginUiIsVisible>(table, "ui_is_visible");
    return host;
  }

  void Api::init(crs::InitType type, Plugin *loaded, std::function<void()> first_initializer, std::function<void()> initializer)
  {
    crs::plugin = loaded;
    crs::api = bind_host_api(loaded->api);
    if (type == crs::InitType::loaded)
    {
      api.event_bus_register(EngineTickEvent::specific_id(), reinterpret_cast<void *>(event_handler_engine_tick), nullptr);
      api.event_bus_register(MenuActionEvent::pre_id(), reinterpret_cast<void *>(event_handler_menu_action), nullptr);
      api.event_bus_register(MenuOpenedEvent::specific_id(), reinterpret_cast<void *>(event_handler_menu_opened), nullptr);
      api.event_bus_register(WorldSettingChangedEvent::specific_id(), reinterpret_cast<void *>(event_handler_world_setting_changed), nullptr);
      api.event_bus_register(ItemChangedEvent::specific_id(), reinterpret_cast<void *>(event_handler_item_changed), nullptr);
      api.event_bus_register(NewChatMessageEvent::specific_id(), reinterpret_cast<void *>(event_handler_new_chat_message), nullptr);
      first_initializer();
    }

    initializer();
  }

  void Api::force_on()
  {
    crs::forced_on = true;
    enabled_flag.store(true, std::memory_order_relaxed);
  }

  std::string Api::configuration_dir()
  {
    if (!api.get_configuration_dir)
    {
      return {};
    }
    auto *dir = api.get_configuration_dir();
    return dir ? dir : "";
  }

  uint64_t Api::root_plugin_component_id()
  {
    return default_ui_parent();
  }

  ApiContainer Api::add_container(uint64_t parent_id)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::container, parent_id);
    return ApiContainer(api, id);
  }

  ApiContainer Api::add_container()
  {
    return add_container(default_ui_parent());
  }

  ApiContainer Api::add_row(uint64_t parent_id)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::row, parent_id);
    return ApiContainer(api, id);
  }

  ApiContainer Api::add_row()
  {
    return add_row(default_ui_parent());
  }

  ApiLabel Api::add_label(uint64_t parent_id, const std::string &text)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::label, parent_id);
    api.ui_update_component_text(id, text.c_str());
    return ApiLabel(api, id);
  }

  ApiLabel Api::add_label(const std::string &text)
  {
    return add_label(default_ui_parent(), text);
  }

  ApiHr Api::add_hr(uint64_t parent_id)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::hr, parent_id);
    return ApiHr(api, id);
  }

  ApiHr Api::add_hr()
  {
    return add_hr(default_ui_parent());
  }

  ApiCheckBox Api::add_checkbox(uint64_t parent_id, const std::string &text)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::checkbox, parent_id);
    api.ui_update_component_text(id, text.c_str());
    return ApiCheckBox(api, id);
  }

  ApiCheckBox Api::add_checkbox(const std::string &text)
  {
    return add_checkbox(default_ui_parent(), text);
  }

  static void dropdown_change_handler(uint64_t id, int32_t selected, void *)
  {
    std::shared_ptr<ApiDropDown> dropdown;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_dropdowns.find(id);
      if (component != crs::ui_dropdowns.end())
      {
        dropdown = component->second;
      }
    }
    if (dropdown)
    {
      dropdown->fire_changed(selected);
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

    api.ui_register_dropdown_change_handler(id, dropdown_change_handler, reinterpret_cast<void *>(id));

    auto dropdown = std::make_shared<ApiDropDown>(api, id);
    {
      std::lock_guard lock(ui_component_maps_mu);
      crs::ui_dropdowns[id] = dropdown;
    }

    return dropdown;
  }

  std::shared_ptr<ApiDropDown> Api::add_dropdown(const std::vector<std::string> &options)
  {
    return add_dropdown(default_ui_parent(), options);
  }

  static void button_click_handler(uint64_t id, void *)
  {
    std::shared_ptr<ApiButton> button;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_buttons.find(id);
      if (component != crs::ui_buttons.end())
      {
        button = component->second;
      }
    }
    if (button)
    {
      button->fire_clicked();
    }
  }

  std::shared_ptr<ApiButton> Api::add_button(uint64_t parent_id, const std::string &text)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::button, parent_id);
    api.ui_update_component_text(id, text.c_str());
    api.ui_register_button_click_handler(id, button_click_handler, reinterpret_cast<void *>(id));

    auto button = std::make_shared<ApiButton>(api, id);
    {
      std::lock_guard lock(ui_component_maps_mu);
      crs::ui_buttons[id] = button;
    }
    return button;
  }

  std::shared_ptr<ApiButton> Api::add_button(const std::string &text)
  {
    return add_button(default_ui_parent(), text);
  }

  static void graph_map_select_handler(uint64_t id, uint32_t node_id, void *)
  {
    std::shared_ptr<ApiGraphMap> map;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_graph_maps.find(id);
      if (component != crs::ui_graph_maps.end())
      {
        map = component->second;
      }
    }
    if (map)
    {
      map->fire_select(node_id);
    }
  }

  static void graph_map_link_handler(uint64_t id, uint32_t from, uint32_t to, void *)
  {
    std::shared_ptr<ApiGraphMap> map;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_graph_maps.find(id);
      if (component != crs::ui_graph_maps.end())
      {
        map = component->second;
      }
    }
    if (map)
    {
      map->fire_link(from, to);
    }
  }

  static void graph_map_object_link_handler(uint64_t id, uint32_t from_vertex, uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option, void *)
  {
    std::shared_ptr<ApiGraphMap> map;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_graph_maps.find(id);
      if (component != crs::ui_graph_maps.end())
      {
        map = component->second;
      }
    }
    if (map)
    {
      map->fire_object_link(from_vertex, object_id, tile_x, tile_y, option);
    }
  }

  static void graph_map_background_handler(uint64_t id, uint32_t tile_x, uint32_t tile_y, void *)
  {
    std::shared_ptr<ApiGraphMap> map;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_graph_maps.find(id);
      if (component != crs::ui_graph_maps.end())
      {
        map = component->second;
      }
    }
    if (map)
    {
      map->fire_background(tile_x, tile_y);
    }
  }

  static void graph_map_context_handler(uint64_t id, uint32_t action, uint32_t tile_x, uint32_t tile_y, int32_t vertex_id, void *)
  {
    std::shared_ptr<ApiGraphMap> map;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_graph_maps.find(id);
      if (component != crs::ui_graph_maps.end())
      {
        map = component->second;
      }
    }
    if (map)
    {
      map->fire_context(action, tile_x, tile_y, vertex_id);
    }
  }

  static void graph_map_zoom_handler(uint64_t id, uint32_t radius_tiles, void *)
  {
    std::shared_ptr<ApiGraphMap> map;
    {
      std::lock_guard lock(ui_component_maps_mu);
      auto component = crs::ui_graph_maps.find(id);
      if (component != crs::ui_graph_maps.end())
      {
        map = component->second;
      }
    }
    if (map)
    {
      map->fire_zoom(radius_tiles);
    }
  }

  std::shared_ptr<ApiGraphMap> Api::add_graph_map(uint64_t parent_id)
  {
    auto id = api.ui_allocate_component(crs::PluginComponentType::graph_map, parent_id);
    api.ui_register_graph_map_select_handler(id, graph_map_select_handler, reinterpret_cast<void *>(id));
    api.ui_register_graph_map_link_handler(id, graph_map_link_handler, reinterpret_cast<void *>(id));
    api.ui_register_graph_map_object_link_handler(id, graph_map_object_link_handler, reinterpret_cast<void *>(id));
    api.ui_register_graph_map_background_handler(id, graph_map_background_handler, reinterpret_cast<void *>(id));
    api.ui_register_graph_map_context_handler(id, graph_map_context_handler, reinterpret_cast<void *>(id));
    api.ui_register_graph_map_zoom_handler(id, graph_map_zoom_handler, reinterpret_cast<void *>(id));

    auto map = std::make_shared<ApiGraphMap>(api, id);
    {
      std::lock_guard lock(ui_component_maps_mu);
      crs::ui_graph_maps[id] = map;
    }
    return map;
  }

  std::shared_ptr<ApiGraphMap> Api::add_graph_map()
  {
    return add_graph_map(default_ui_parent());
  }

  Globals *Api::raw_globals()
  {
    return api.get_globals().unwrap();
  }

  Engine *Api::raw_engine()
  {
    auto globals = raw_globals();
    if (!globals)
    {
      return nullptr;
    }

    return globals->engine;
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
    if (idx < 0 || static_cast<size_t>(idx) >= player_update_cache->updates.size())
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
      players.reserve(static_cast<size_t>(cache->updates.size()));
      for (auto it = cache->updates.begin(); it != cache->updates.end(); it++)
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
      npcs.reserve(static_cast<size_t>(cache->size));
      for (uint64_t i = 0; i < cache->size; i++)
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

  SocialCache *Api::raw_social_cache()
  {
    auto engine = Api::raw_engine();
    if (!engine)
    {
      return nullptr;
    }

    return engine->social_cache;
  }

  bool Api::raw_is_friend(const Player *player)
  {
    if (!player)
    {
      return false;
    }

    auto cache = Api::raw_social_cache();
    if (!cache)
    {
      return false;
    }

    for (auto i = cache->friends.begin(); i != cache->friends.end(); i++)
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

  LocalPlayerVariables *Api::raw_local_player_variables()
  {
    auto engine = Api::raw_engine();
    if (!engine)
    {
      return nullptr;
    }

    auto vc = engine->variable_cache;
    if (!vc)
    {
      return nullptr;
    }

    return vc->local_player_variables;
  }

  std::optional<ApiPlayer> Api::self()
  {
    auto rs = Api::raw_self();
    if (!rs)
    {
      return std::nullopt;
    }

    return ApiPlayer(rs);
  }

  std::optional<Vec2<uint32_t>> Api::self_tile()
  {
    if (auto s = self())
    {
      return s->tile_position();
    }
    return std::nullopt;
  }

  std::vector<ApiPlayer> Api::players(std::function<bool(const ApiPlayer &)> conditional)
  {
    std::vector<ApiPlayer> out;
    detail::for_each_player([&](ApiPlayer player)
    {
      if (conditional(player))
      {
        out.push_back(std::move(player));
      }
      return true;
    });
    return out;
  }

  std::vector<ApiNpc> Api::npcs(std::function<bool(const ApiNpc &)> conditional)
  {
    std::vector<ApiNpc> out;
    detail::for_each_npc([&](ApiNpc npc)
    {
      if (conditional(npc))
      {
        out.push_back(std::move(npc));
      }
      return true;
    });
    return out;
  }

  std::vector<ApiObject> Api::objects(std::function<bool(const ApiObject &)> conditional)
  {
    std::vector<ApiObject> out;
    detail::for_each_object([&](ApiObject obj)
    {
      if (conditional(obj))
      {
        out.push_back(std::move(obj));
      }
      return true;
    });
    return out;
  }

  std::optional<ApiPlayer> Api::find_player(std::function<bool(const ApiPlayer &)> conditional)
  {
    std::optional<ApiPlayer> found;
    detail::for_each_player([&](ApiPlayer player)
    {
      if (conditional(player))
      {
        found = std::move(player);
        return false;
      }
      return true;
    });
    return found;
  }

  std::optional<ApiNpc> Api::find_npc(std::function<bool(const ApiNpc &)> conditional)
  {
    std::optional<ApiNpc> found;
    detail::for_each_npc([&](ApiNpc npc)
    {
      if (conditional(npc))
      {
        found = std::move(npc);
        return false;
      }
      return true;
    });
    return found;
  }

  std::optional<ApiObject> Api::find_object(std::function<bool(const ApiObject &)> conditional)
  {
    std::optional<ApiObject> found;
    detail::for_each_object([&](ApiObject obj)
    {
      if (conditional(obj))
      {
        found = std::move(obj);
        return false;
      }
      return true;
    });
    return found;
  }

  std::optional<ApiPlayer> Api::closest_player(std::function<bool(const ApiPlayer &)> conditional, uint32_t max_dist)
  {
    auto from = self_tile();
    if (!from)
    {
      return std::nullopt;
    }
    return closest_player(*from, std::move(conditional), max_dist);
  }

  std::optional<ApiPlayer> Api::closest_player(Vec2<uint32_t> from, std::function<bool(const ApiPlayer &)> conditional, uint32_t max_dist)
  {
    return detail::closest_from<ApiPlayer>([&](auto &&accept)
    {
      detail::for_each_player([&](ApiPlayer player)
      {
        if (conditional(player))
        {
          accept(std::move(player));
        }
        return true;
      });
    },
                                           from, max_dist);
  }

  std::optional<ApiNpc> Api::closest_npc(std::function<bool(const ApiNpc &)> conditional, uint32_t max_dist)
  {
    auto from = self_tile();
    if (!from)
    {
      return std::nullopt;
    }
    return closest_npc(*from, std::move(conditional), max_dist);
  }

  std::optional<ApiNpc> Api::closest_npc(Vec2<uint32_t> from, std::function<bool(const ApiNpc &)> conditional, uint32_t max_dist)
  {
    return detail::closest_from<ApiNpc>([&](auto &&accept)
    {
      detail::for_each_npc([&](ApiNpc npc)
      {
        if (conditional(npc))
        {
          accept(std::move(npc));
        }
        return true;
      });
    },
                                         from, max_dist);
  }

  std::optional<ApiObject> Api::closest_object(std::function<bool(const ApiObject &)> conditional, uint32_t max_dist)
  {
    auto from = self_tile();
    if (!from)
    {
      return std::nullopt;
    }
    return closest_object(*from, std::move(conditional), max_dist);
  }

  std::optional<ApiObject> Api::closest_object(Vec2<uint32_t> from, std::function<bool(const ApiObject &)> conditional, uint32_t max_dist)
  {
    return detail::closest_from<ApiObject>([&](auto &&accept)
    {
      detail::for_each_object([&](ApiObject obj)
      {
        if (conditional(obj))
        {
          accept(std::move(obj));
        }
        return true;
      });
    },
                                           from, max_dist);
  }

  std::optional<ApiObject> Api::closest_object_id(uint32_t object_id, uint32_t max_dist)
  {
    return closest_object([object_id](const ApiObject &obj)
    {
      return obj.matches_id(object_id);
    },
                          max_dist);
  }

  std::optional<ApiObject> Api::closest_object_id(Vec2<uint32_t> from, uint32_t object_id, uint32_t max_dist)
  {
    return closest_object(from, [object_id](const ApiObject &obj)
    {
      return obj.matches_id(object_id);
    },
                          max_dist);
  }

  uint32_t Api::chebyshev(uint32_t ax, uint32_t ay, uint32_t bx, uint32_t by)
  {
    auto dx = ax > bx ? ax - bx : bx - ax;
    auto dy = ay > by ? ay - by : by - ay;
    return dx > dy ? dx : dy;
  }

  uint32_t Api::chebyshev(Vec2<uint32_t> a, Vec2<uint32_t> b)
  {
    return chebyshev(a.x, a.y, b.x, b.y);
  }

  bool Api::same_plane(const ApiEntity &entity)
  {
    if (auto s = self())
    {
      return entity.plane() == s->plane();
    }
    return false;
  }

  uint32_t Api::get_world_setting(uint32_t id)
  {
    auto cache = Api::raw_world_setting_cache();
    if (!cache || !cache->vars || cache->count == 0)
    {
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

    return 0;
  }

  std::optional<ApiItemContainer> Api::get_item_container(uint32_t id, uint16_t parent_widget, uint16_t child_widget, uint32_t fallback_capacity)
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

    for (auto i = item_cache->containers.begin(); i != item_cache->containers.end(); i++)
    {
      if (i->id == id)
      {
        std::vector<ApiItem> items;

        auto slot = 0;
        for (auto item = i->items.begin(); item != i->items.end(); item++)
        {
          if (item->id != -1)
          {
            items.push_back(ApiItem(parent_widget, child_widget, slot, item->id, item->amount));
          }

          slot += 1;
        }

        const auto capacity = i->items.size() != 0 ? static_cast<uint32_t>(i->items.size()) : fallback_capacity;
        return ApiItemContainer(id, capacity, items);
      }
    }

    if (fallback_capacity != 0)
    {
      return ApiItemContainer(id, fallback_capacity, {});
    }

    return std::optional<ApiItemContainer>();
  }

  std::optional<ApiItemContainer> Api::get_inventory()
  {
    return Api::get_item_container(Containers::inventory, InventoryWidget::parent, InventoryWidget::child, 28);
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
      off(Globals, menu_action_handler_obj0),
      off(Globals, menu_action_handler_obj1),
      off(Globals, menu_action_handler_obj2),
      off(Globals, menu_action_handler_obj3),
      off(Globals, menu_action_handler_obj4),
      off(Globals, menu_action_handler_obj5),
      off(Globals, menu_action_handler_obj6)
    };

    static uint64_t npc_offsets[] = {
      off(Globals, menu_action_handler_npc0),
      off(Globals, menu_action_handler_npc1),
      off(Globals, menu_action_handler_npc2),
      off(Globals, menu_action_handler_npc3),
      off(Globals, menu_action_handler_npc4),
      off(Globals, menu_action_handler_npc5),
      off(Globals, menu_action_handler_npc6)
    };

    static uint64_t widget_offsets[] = {
      off(Globals, menu_action_handler_widget0),
      off(Globals, menu_action_handler_widget1),
      off(Globals, menu_action_handler_widget2)
    };

    auto globals = Api::raw_globals();
    if (!globals)
    {
      return nullptr;
    }

    if (type == MenuActionType::walk)
    {
      return reinterpret_cast<FnMenuActionHandler>(&globals->menu_action_handler_walk);
    }
    else if (type == MenuActionType::obj && idx < std::size(obj_offsets))
    {
      return reinterpret_cast<FnMenuActionHandler>(reinterpret_cast<char *>(globals) + obj_offsets[idx]);
    }
    else if (type == MenuActionType::npc && idx < std::size(npc_offsets))
    {
      return reinterpret_cast<FnMenuActionHandler>(reinterpret_cast<char *>(globals) + npc_offsets[idx]);
    }
    else if (type == MenuActionType::widget && idx < std::size(widget_offsets))
    {
      return reinterpret_cast<FnMenuActionHandler>(reinterpret_cast<char *>(globals) + widget_offsets[idx]);
    }

    return nullptr;
  }

  void Api::perform_menu_action(FnMenuActionHandler handler, const MenuActionArgs &args)
  {
    if (!handler)
    {
      return;
    }

    MenuActionTemplate templ{};
    templ.engine = Api::raw_engine();
    templ.handler = handler;

    MenuActionContext ctx{};
    ctx.tmpl = &templ;
    ctx.args = args;

    MenuAction am_ctx{};
    am_ctx.menu_action_context = &ctx;

    handler(&templ, &am_ctx);
  }

  void Api::select_item(uint16_t parent_widget, uint16_t child_widget, int32_t slot)
  {
    MenuActionArgs args;
    args.args_widget.option_idx = 0;
    args.args_widget.sub_idx = slot;
    args.args_widget.widget_id = (static_cast<uint32_t>(parent_widget) << 16) | child_widget;
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

  uint64_t Api::on_tick(std::function<void()> f, EventGate when)
  {
    return crs::tick_events.reg(std::move(f), std::move(when));
  }

  uint64_t Api::on_menu_action(std::function<void(MenuActionEventArgs *)> f, EventGate when)
  {
    return crs::menu_action_events.reg(std::move(f), std::move(when));
  }

  uint64_t Api::on_menu_opened(std::function<void(bool)> f, EventGate when)
  {
    return crs::menu_opened_events.reg(std::move(f), std::move(when));
  }

  uint64_t Api::on_world_setting_changed(std::function<void(uint32_t, uint32_t)> f, EventGate when)
  {
    return crs::world_setting_changed_events.reg(std::move(f), std::move(when));
  }

  uint64_t Api::on_item_changed(std::function<void(uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t)> f, EventGate when)
  {
    return crs::item_changed_events.reg(std::move(f), std::move(when));
  }

  uint64_t Api::on_new_chat_message(std::function<void(const std::string &, const std::string &, const std::string &)> f, EventGate when)
  {
    return crs::new_chat_message_events.reg(std::move(f), std::move(when));
  }

  void Api::expose(const std::string &name, void *function)
  {
    crs::api.expose_function(name.c_str(), reinterpret_cast<FnPluginExposedFunction>(function), nullptr);
  }

  void *Api::exposed(const std::string &name)
  {
    return reinterpret_cast<void *>(crs::api.get_exposed_function(name.c_str()));
  }

  void Api::log(const std::string &s)
  {
    crs::api.log(s.c_str());
  }

  static void init_plugin_ui(std::function<void()> plugin_ui)
  {
    enabled_checkbox = Api::add_checkbox(plugin->ui_tab_container_id, "Enabled");
    enabled_flag.store(forced_on, std::memory_order_relaxed);
    if (forced_on)
    {
      enabled_checkbox.set_checked(true);
      enabled_checkbox.set_visible(false);
    }

    plugin_content = Api::add_container(plugin->ui_tab_container_id);
    plugin_content_visible = !is_enabled();
    sync_plugin_content_visibility();

    plugin_ui();
  }

  static void plugin_toggle(bool active)
  {
    enabled_flag.store(active, std::memory_order_relaxed);
    enabled_checkbox.set_checked(active);
    sync_plugin_content_visibility();
  }
} // namespace crs

PLUGIN_API
uint32_t plugin_api_version()
{
  return VERSION_API;
}

PLUGIN_API
const char *plugin_get_name()
{
  static char plugin_name_cached[256];
  std::strcpy(plugin_name_cached, crs::Boot::name().c_str());
  return plugin_name_cached;
}

PLUGIN_API
const char *plugin_get_version()
{
  static char plugin_version_cached[64];
  std::strcpy(plugin_version_cached, crs::Boot::version().c_str());
  return plugin_version_cached;
}

PLUGIN_API
void plugin_init(crs::InitType type, crs::Plugin *plugin)
{
  crs::Api::init(type, plugin, []()
  {
    crs::Boot::init();
    crs::Api::expose(crs::Boot::name() + "_toggle", reinterpret_cast<void *>(crs::plugin_toggle));
  }, []()
  {
    crs::init_plugin_ui([]()
    {
      crs::Boot::init_ui();
    });
  });
}
