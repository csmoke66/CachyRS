#include "plugin.h"
#include "cachy.h"
#include "log.h"
#include "plugin_api_validate.h"
#include "version.hpp"

#include <dlfcn.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace crs
{
  namespace v = plugin_api_validate;

  static std::unordered_map<std::string, FnPluginExposedFunction> exposed_functions;
  static std::unordered_map<std::string, void *> api_functions;

  class CEventBusReceiver : public EventReceiver<Event>
  {
  private:
    FnPluginEventBusReceiver receiver;
    void *context;

  public:
    CEventBusReceiver(FnPluginEventBusReceiver receiver, void *context) : receiver(receiver),
                                                                          context(context)
    {
    }

    void receive(Event *event) override
    {
      receiver(event->get_args(), context);
    }
  };

  static bool require_ui(const char *api)
  {
    if (!RS.ui)
    {
      v::fatal(api, "UI not initialized");
    }
    return true;
  }

  static bool require_live_component(const char *api, uint64_t component_id)
  {
    if (!v::component_id_sane(api, component_id))
    {
      return false;
    }
    require_ui(api);
    const bool exists = RS.ui_locked([component_id]()
    {
      return RS.ui->has_component(component_id);
    });
    if (!exists)
    {
      v::soft_fail(api, "unknown component id", component_id);
      return false;
    }
    return true;
  }

  static bool require_live_component_type(const char *api, uint64_t component_id, ComponentType type)
  {
    if (!require_live_component(api, component_id))
    {
      return false;
    }
    const bool ok = RS.ui_locked([component_id, type]()
    {
      return RS.ui->component_is_type(component_id, type);
    });
    if (!ok)
    {
      v::soft_fail(api, "component id has wrong type", component_id);
      return false;
    }
    return true;
  }

  static void plugin_api_log(const char *message)
  {
    v::optional_readable("log", message, 1);
    if (!message)
    {
      v::soft_fail("log", "null message");
      return;
    }
    v::require_c_string("log", "message", message);
    LOG(PLUGIN, message);
  }

  static std::string plugin_configuration_dir;

  static const char *plugin_api_get_configuration_dir()
  {
    return plugin_configuration_dir.c_str();
  }

  static ThreadOwned<Globals *> plugin_api_get_globals()
  {
    return RS.get_globals();
  }

  static uint64_t plugin_api_user_interface_allocate_component(PluginComponentType type, uint64_t parent_id)
  {
    constexpr auto api = "ui_allocate_component";
    v::require_enum_range(api, type, PluginComponentType::row);
    if (!v::component_id_sane(api, parent_id))
    {
      return 0;
    }
    require_ui(api);
    if (!RS.ui_locked([parent_id]()
    {
      return RS.ui->has_component(parent_id);
    }))
    {
      v::soft_fail(api, "unknown parent component id", parent_id);
      return 0;
    }

    return RS.ui_locked([type, parent_id]()
    {
      return RS.ui->allocate_component(static_cast<ComponentType>(type), parent_id);
    });
  }

  static void plugin_api_user_interface_update_component_text(uint64_t component_id, const char *text)
  {
    constexpr auto api = "ui_update_component_text";
    if (!require_live_component(api, component_id))
    {
      return;
    }
    v::optional_readable(api, text, 1);
    if (text)
    {
      v::require_c_string(api, "text", text);
    }

    std::string copy(text ? text : "");
    RS.ui_locked_nr([component_id, &copy]()
    {
      RS.ui->update_component_text(component_id, copy);
    });
  }

  static void plugin_api_user_interface_update_component_items(uint64_t component_id, const char **items, size_t item_count)
  {
    constexpr auto api = "ui_update_component_items";
    if (!require_live_component_type(api, component_id, ComponentType::dropdown))
    {
      return;
    }
    v::require_collection_count(api, item_count, items, sizeof(const char *));
    if (item_count > 0)
    {
      for (size_t i = 0; i < item_count; i++)
      {
        v::optional_readable(api, items[i], 1);
        if (items[i])
        {
          v::require_c_string(api, "items[i]", items[i]);
        }
      }
    }

    std::vector<std::string> converted;
    if (items)
    {
      converted.reserve(item_count);
      for (size_t i = 0; i < item_count; i++)
      {
        converted.emplace_back(items[i] ? items[i] : "");
      }
    }

    RS.ui_locked_nr([component_id, &converted]()
    {
      RS.ui->update_component_items(component_id, converted);
    });
  }

  static bool plugin_api_user_interface_is_component_active(uint64_t component_id)
  {
    constexpr auto api = "ui_is_component_active";
    if (!require_live_component(api, component_id))
    {
      return false;
    }
    return RS.ui_locked([component_id]()
    {
      return RS.ui->is_component_active(component_id);
    });
  }

  static void plugin_api_user_interface_set_component_active(uint64_t component_id, bool active)
  {
    constexpr auto api = "ui_set_component_active";
    if (!require_live_component(api, component_id))
    {
      return;
    }
    RS.ui_locked_nr([component_id, active]()
    {
      RS.ui->set_component_active(component_id, active);
    });
  }

  static void plugin_api_user_interface_register_dropdown_change_handler(uint64_t component_id, FnPluginUserInterfaceDropDownChangeHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_dropdown_change_handler";
    if (!require_live_component_type(api, component_id, ComponentType::dropdown))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_dropdown_change_handler(component_id, [component_id, handler, user_data](int index)
      {
        handler(component_id, index, user_data);
      });
    });
  }

  static void plugin_api_user_interface_register_button_click_handler(uint64_t component_id, FnPluginUserInterfaceButtonClickHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_button_click_handler";
    if (!require_live_component_type(api, component_id, ComponentType::button))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_button_click_handler(component_id, [component_id, handler, user_data]()
      {
        handler(component_id, user_data);
      });
    });
  }

  static void plugin_api_user_interface_dropdown_set_selected(uint64_t component_id, int32_t index)
  {
    constexpr auto api = "ui_dropdown_set_selected";
    if (!require_live_component_type(api, component_id, ComponentType::dropdown))
    {
      return;
    }
    if (index < -1)
    {
      v::soft_fail(api, "dropdown index < -1", static_cast<uintptr_t>(index));
      return;
    }
    RS.ui_locked_nr([component_id, index]()
    {
      RS.ui->dropdown_set_selected(component_id, index);
    });
  }

  static void plugin_api_user_interface_set_visible(uint64_t component_id, bool visible)
  {
    constexpr auto api = "ui_set_visible";
    if (!require_live_component(api, component_id))
    {
      return;
    }
    RS.ui_locked_nr([component_id, visible]()
    {
      RS.ui->set_component_visible(component_id, visible);
    });
  }

  static void plugin_api_user_interface_update_graph_map(uint64_t component_id, uint32_t center_x, uint32_t center_y, uint32_t radius_tiles,
      uint32_t selected_id, const PluginGraphMapNode *nodes, size_t node_count,
      const PluginGraphMapEdge *edges, size_t edge_count,
      const PluginGraphMapObject *objects, size_t object_count)
  {
    constexpr auto api = "ui_update_graph_map";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    if (radius_tiles == 0 || radius_tiles > 10'000)
    {
      v::soft_fail(api, "radius_tiles out of range", radius_tiles);
      return;
    }
    v::require_collection_count(api, node_count, nodes, sizeof(PluginGraphMapNode));
    v::require_collection_count(api, edge_count, edges, sizeof(PluginGraphMapEdge));
    v::require_collection_count(api, object_count, objects, sizeof(PluginGraphMapObject));

    std::vector<GraphMapNode> converted_nodes;
    converted_nodes.reserve(node_count);
    for (size_t i = 0; i < node_count; i++)
    {
      converted_nodes.push_back(GraphMapNode{ nodes[i].id, nodes[i].x, nodes[i].y, nodes[i].flags });
    }

    std::vector<GraphMapEdge> converted_edges;
    converted_edges.reserve(edge_count);
    for (size_t i = 0; i < edge_count; i++)
    {
      converted_edges.push_back(GraphMapEdge{ edges[i].from, edges[i].to, edges[i].kind });
    }

    std::vector<GraphMapObject> converted_objects;
    converted_objects.reserve(object_count);
    for (size_t i = 0; i < object_count; i++)
    {
      GraphMapObject obj{};
      obj.object_id = objects[i].object_id;
      obj.x = objects[i].x;
      obj.y = objects[i].y;
      obj.option = objects[i].option;
      obj.name = objects[i].name;
      converted_objects.push_back(std::move(obj));
    }

    RS.ui_locked_nr([component_id, center_x, center_y, radius_tiles, selected_id, converted_nodes = std::move(converted_nodes),
                        converted_edges = std::move(converted_edges), converted_objects = std::move(converted_objects)]() mutable
    {
      RS.ui->update_graph_map(component_id, center_x, center_y, radius_tiles, selected_id, converted_nodes, converted_edges, converted_objects);
    });
  }

  static void plugin_api_user_interface_update_graph_map_primitives(uint64_t component_id, const PluginGraphMapPrimitive *primitives, size_t primitive_count)
  {
    constexpr auto api = "ui_update_graph_map_primitives";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_collection_count(api, primitive_count, primitives, sizeof(PluginGraphMapPrimitive));

    std::vector<GraphMapPrimitive> converted;
    converted.reserve(primitive_count);
    for (size_t i = 0; i < primitive_count; i++)
    {
      GraphMapPrimitive p{};
      p.kind = primitives[i].kind == static_cast<uint8_t>(PluginGraphMapPrimitiveKind::disc)
                   ? GraphMapPrimitiveKind::disc
                   : GraphMapPrimitiveKind::line;
      p.r = primitives[i].r;
      p.g = primitives[i].g;
      p.b = primitives[i].b;
      p.a = primitives[i].a;
      p.thickness = primitives[i].thickness;
      p.x0 = primitives[i].x0;
      p.y0 = primitives[i].y0;
      p.x1 = primitives[i].x1;
      p.y1 = primitives[i].y1;
      converted.push_back(p);
    }

    RS.ui_locked_nr([component_id, converted = std::move(converted)]() mutable
    {
      RS.ui->update_graph_map_primitives(component_id, converted);
    });
  }

  static void plugin_api_user_interface_register_graph_map_select_handler(uint64_t component_id, FnPluginUserInterfaceGraphMapSelectHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_graph_map_select_handler";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_graph_map_select_handler(component_id, [component_id, handler, user_data](uint32_t node_id)
      {
        handler(component_id, node_id, user_data);
      });
    });
  }

  static void plugin_api_user_interface_register_graph_map_link_handler(uint64_t component_id, FnPluginUserInterfaceGraphMapLinkHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_graph_map_link_handler";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_graph_map_link_handler(component_id, [component_id, handler, user_data](uint32_t from, uint32_t to)
      {
        handler(component_id, from, to, user_data);
      });
    });
  }

  static void plugin_api_user_interface_register_graph_map_object_link_handler(uint64_t component_id, FnPluginUserInterfaceGraphMapObjectLinkHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_graph_map_object_link_handler";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_graph_map_object_link_handler(component_id, [component_id, handler, user_data](uint32_t from, uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option)
      {
        handler(component_id, from, object_id, tile_x, tile_y, option, user_data);
      });
    });
  }

  static void plugin_api_user_interface_register_graph_map_background_handler(uint64_t component_id, FnPluginUserInterfaceGraphMapBackgroundHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_graph_map_background_handler";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_graph_map_background_handler(component_id, [component_id, handler, user_data](uint32_t tile_x, uint32_t tile_y)
      {
        handler(component_id, tile_x, tile_y, user_data);
      });
    });
  }

  static void plugin_api_user_interface_register_graph_map_context_handler(uint64_t component_id, FnPluginUserInterfaceGraphMapContextHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_graph_map_context_handler";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_graph_map_context_handler(component_id, [component_id, handler, user_data](uint32_t action, uint32_t tile_x, uint32_t tile_y, int32_t vertex_id)
      {
        handler(component_id, action, tile_x, tile_y, vertex_id, user_data);
      });
    });
  }

  static void plugin_api_user_interface_register_graph_map_zoom_handler(uint64_t component_id, FnPluginUserInterfaceGraphMapZoomHandler handler, void *user_data)
  {
    constexpr auto api = "ui_register_graph_map_zoom_handler";
    if (!require_live_component_type(api, component_id, ComponentType::graph_map))
    {
      return;
    }
    v::require_fn_ptr(api, "handler", reinterpret_cast<const void *>(handler));
    v::opaque_cookie_sane(api, user_data);

    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_graph_map_zoom_handler(component_id, [component_id, handler, user_data](uint32_t radius_tiles)
      {
        handler(component_id, radius_tiles, user_data);
      });
    });
  }

  static void plugin_api_event_bus_register(const char *id, FnPluginEventBusReceiver receiver, void *context)
  {
    constexpr auto api = "event_bus_register";
    v::require_c_string(api, "id", id);
    v::require_fn_ptr(api, "receiver", reinterpret_cast<const void *>(receiver));
    v::opaque_cookie_sane(api, context);
    RS.event_bus.add_receiver(id, new CEventBusReceiver(receiver, context));
  }

  static void plugin_api_expose_function(const char *name, FnPluginExposedFunction fn, void *context)
  {
    constexpr auto api = "expose_function";
    v::require_c_string(api, "name", name);
    v::require_fn_ptr(api, "fn", reinterpret_cast<const void *>(fn));
    v::opaque_cookie_sane(api, context);
    (void)context;
    exposed_functions[name] = fn;
  }

  static FnPluginExposedFunction plugin_api_get_exposed_function(const char *name)
  {
    constexpr auto api = "get_exposed_function";
    v::require_c_string(api, "name", name);
    auto it = exposed_functions.find(name);
    return it == exposed_functions.end() ? nullptr : it->second;
  }

  static bool plugin_api_ui_is_visible()
  {
    return RS.ui_visible.load(std::memory_order_relaxed);
  }

  static bool plugin_api_widget_is_visible(const void *widget)
  {
    if (!widget)
    {
      return false;
    }

    // Render-cache keys only — do not dereference the pointer.
    if (auto *rw = RS.hook_manager->view_hook<RenderWidgetHook>("render_widget"))
    {
      return rw->is_visible(reinterpret_cast<const Widget *>(widget));
    }
    return false;
  }

  static void *plugin_api_resolve(const char *name)
  {
    if (!name)
    {
      v::soft_fail("resolve", "null name");
      return nullptr;
    }
    const auto u = reinterpret_cast<uintptr_t>(name);
    if (u < v::k_user_va_min || u > v::k_user_va_max)
    {
      v::fatal("resolve", "name pointer outside user address space", u);
    }
    v::require_c_string("resolve", "name", name);

    auto it = api_functions.find(name);
    return it == api_functions.end() ? nullptr : it->second;
  }

  static void register_api(const char *name, void *fn)
  {
    api_functions[name] = fn;
  }

  void PluginManager::init()
  {
    plugin_configuration_dir = RS.get_configuration_dir();
    api_functions.clear();

    register_api("log", reinterpret_cast<void *>(plugin_api_log));
    register_api("get_configuration_dir", reinterpret_cast<void *>(plugin_api_get_configuration_dir));
    register_api("get_globals", reinterpret_cast<void *>(plugin_api_get_globals));

    register_api("ui_allocate_component", reinterpret_cast<void *>(plugin_api_user_interface_allocate_component));
    register_api("ui_update_component_text", reinterpret_cast<void *>(plugin_api_user_interface_update_component_text));
    register_api("ui_update_component_items", reinterpret_cast<void *>(plugin_api_user_interface_update_component_items));
    register_api("ui_is_component_active", reinterpret_cast<void *>(plugin_api_user_interface_is_component_active));
    register_api("ui_set_component_active", reinterpret_cast<void *>(plugin_api_user_interface_set_component_active));
    register_api("ui_register_dropdown_change_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_dropdown_change_handler));
    register_api("ui_register_button_click_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_button_click_handler));
    register_api("ui_dropdown_set_selected", reinterpret_cast<void *>(plugin_api_user_interface_dropdown_set_selected));
    register_api("ui_set_visible", reinterpret_cast<void *>(plugin_api_user_interface_set_visible));
    register_api("ui_update_graph_map", reinterpret_cast<void *>(plugin_api_user_interface_update_graph_map));
    register_api("ui_update_graph_map_primitives", reinterpret_cast<void *>(plugin_api_user_interface_update_graph_map_primitives));
    register_api("ui_register_graph_map_select_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_graph_map_select_handler));
    register_api("ui_register_graph_map_link_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_graph_map_link_handler));
    register_api("ui_register_graph_map_object_link_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_graph_map_object_link_handler));
    register_api("ui_register_graph_map_background_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_graph_map_background_handler));
    register_api("ui_register_graph_map_context_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_graph_map_context_handler));
    register_api("ui_register_graph_map_zoom_handler", reinterpret_cast<void *>(plugin_api_user_interface_register_graph_map_zoom_handler));

    register_api("event_bus_register", reinterpret_cast<void *>(plugin_api_event_bus_register));

    register_api("expose_function", reinterpret_cast<void *>(plugin_api_expose_function));
    register_api("get_exposed_function", reinterpret_cast<void *>(plugin_api_get_exposed_function));
    register_api("ui_is_visible", reinterpret_cast<void *>(plugin_api_ui_is_visible));
    register_api("widget_is_visible", reinterpret_cast<void *>(plugin_api_widget_is_visible));

    api.version = VERSION_API;
    api.resolve = plugin_api_resolve;
  }

  void PluginManager::add_load_callback(std::function<void(Plugin *)> function)
  {
    plugin_load_callbacks.push_back(function);
  }

#ifdef __linux__
#define REQUIRED_EXTENSION ".so"

  void PluginManager::load(const std::string &path)
  {
    auto handle = dlopen(path.c_str(), RTLD_NOW);
    if (!handle)
    {
      auto error = dlerror();
      LOG(ERROR, "Failed to load plugin at '" << path << "': " << (error ? error : "unknown dlopen error"));
      return;
    }

    auto api_version = reinterpret_cast<FnPluginApiVersion>(dlsym(handle, "plugin_api_version"));
    if (!api_version)
    {
      LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_api_version' (stale ABI) — skipped");
      dlclose(handle);
      return;
    }

    const auto built = api_version();
    if (built != VERSION_API)
    {
      LOG(ERROR, "Plugin at '" << path << "' built against API " << built
                               << ", host is " << VERSION_API << " — skipped");
      dlclose(handle);
      return;
    }

    auto get_name = reinterpret_cast<FnPluginGetName>(dlsym(handle, "plugin_get_name"));
    if (!get_name)
    {
      LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_get_name'");
      dlclose(handle);
      return;
    }

    auto get_version = reinterpret_cast<FnPluginGetVersion>(dlsym(handle, "plugin_get_version"));
    if (!get_version)
    {
      LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_get_version'");
      dlclose(handle);
      return;
    }

    auto init = reinterpret_cast<FnPluginInit>(dlsym(handle, "plugin_init"));
    if (!init)
    {
      LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_init'");
      dlclose(handle);
      return;
    }

    auto name = get_name();
    auto version = get_version();

    auto new_plugin = std::make_unique<Plugin>();
    new_plugin->name = name;
    new_plugin->version = version ? version : "";
    new_plugin->get_name = get_name;
    new_plugin->get_version = get_version;
    new_plugin->init = init;
    new_plugin->api = api;

    for (auto &function : plugin_load_callbacks)
    {
      function(new_plugin.get());
    }

    init(InitType::loaded, new_plugin.get());

    LOG(INFO, "Loaded plugin '" << new_plugin->name << "' v" << new_plugin->version << " at '" + path << "'");
    plugins.push_back(std::move(new_plugin));
  }
#else
  UNSUPPORTED_OS();
#endif

  void PluginManager::load_all(const std::string &path)
  {
    if (std::filesystem::exists(path) &&
        std::filesystem::is_directory(path))
    {
      for (auto &entry : std::filesystem::directory_iterator(path))
      {
        if (std::filesystem::is_regular_file(entry) &&
            entry.path().extension() == REQUIRED_EXTENSION)
        {
          load(entry.path().string());
        }
      }
    }
    else
    {
      LOG(ERROR, "Plugin directory '" << path << "' is invalid");
    }
  }

  const std::vector<std::unique_ptr<Plugin>> &PluginManager::view_plugins() const
  {
    return plugins;
  }
} // namespace crs