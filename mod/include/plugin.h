#pragma once
#include <functional>
#include <memory>
#include <vector>

#include "reversed/reversed.h"

#include "ownership.h"

#define PLUGIN_API __attribute__((visibility("default"))) extern "C"

namespace crs
{
  enum class PluginComponentType
  {
    container,
    label,
    hr,
    line,
    button,
    checkbox,
    dropdown,
    graph_map,
    row,
  };

  struct PluginGraphMapNode
  {
    uint32_t id;
    uint32_t x;
    uint32_t y;
    uint8_t flags;
    uint8_t pad[3];
  };

  struct PluginGraphMapEdge
  {
    uint32_t from;
    uint32_t to;
    uint8_t kind;
    uint8_t pad[3];
  };

  struct PluginGraphMapObject
  {
    uint32_t object_id;
    uint32_t x;
    uint32_t y;
    uint32_t option;
    char name[64];
  };

  enum class PluginGraphMapPrimitiveKind : uint8_t
  {
    line = 0,
    disc = 1,
  };

  struct PluginGraphMapPrimitive
  {
    uint8_t kind;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    uint8_t pad[3];
    float thickness;
    float x0;
    float y0;
    float x1;
    float y1;
  };

  enum class PluginGraphMapContextAction : uint32_t
  {
    add_here = 0,
    add_at_player = 1,
    delete_vertex = 2,
  };

  enum class InitType
  {
    loaded,
    refreshed
  };

  struct Plugin;

  using FnPluginGetName = const char *(*)();
  using FnPluginGetVersion = const char *(*)();
  using FnPluginApiVersion = uint32_t (*)();
  using FnPluginInit = void (*)(InitType type, Plugin *plugin);
  using FnPluginLog = void (*)(const char *message);
  using FnPluginGetConfigurationDir = const char *(*)();
  using FnPluginExposedFunction = void (*)(void *context);

  using FnPluginUserInterfaceDropDownChangeHandler = void (*)(uint64_t id, int32_t selected, void *user_data);
  using FnPluginUserInterfaceButtonClickHandler = void (*)(uint64_t id, void *user_data);
  using FnPluginUserInterfaceGraphMapSelectHandler = void (*)(uint64_t id, uint32_t node_id, void *user_data);
  using FnPluginUserInterfaceGraphMapLinkHandler = void (*)(uint64_t id, uint32_t from, uint32_t to, void *user_data);
  using FnPluginUserInterfaceGraphMapObjectLinkHandler = void (*)(uint64_t id, uint32_t from_vertex, uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option, void *user_data);
  using FnPluginUserInterfaceGraphMapBackgroundHandler = void (*)(uint64_t id, uint32_t tile_x, uint32_t tile_y, void *user_data);
  using FnPluginUserInterfaceGraphMapContextHandler = void (*)(uint64_t id, uint32_t action, uint32_t tile_x, uint32_t tile_y, int32_t vertex_id, void *user_data);
  using FnPluginUserInterfaceGraphMapZoomHandler = void (*)(uint64_t id, uint32_t radius_tiles, void *user_data);

  using FnPluginGetGlobals = ThreadOwned<Globals *> (*)();

  using FnPluginUserInterfaceAllocateComponent = uint64_t (*)(PluginComponentType type, uint64_t parent_id);
  using FnPluginUserInterfaceUpdateComponentText = void (*)(uint64_t component_id, const char *text);
  using FnPluginUserInterfaceUpdateComponentItems = void (*)(uint64_t component_id, const char **items, size_t item_count);
  using FnPluginUserInterfaceIsComponentActive = bool (*)(uint64_t component_id);
  using FnPluginUserInterfaceRegisterDropDownChangeHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceDropDownChangeHandler handler, void *user_data);
  using FnPluginUserInterfaceRegisterButtonClickHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceButtonClickHandler handler, void *user_data);
  using FnPluginUserInterfaceDropDownSetSelected = void (*)(uint64_t component_id, int32_t index);
  using FnPluginUserInterfaceSetVisible = void (*)(uint64_t component_id, bool visible);
  using FnPluginUserInterfaceSetActive = void (*)(uint64_t component_id, bool active);
  using FnPluginUserInterfaceUpdateGraphMap = void (*)(uint64_t component_id, uint32_t center_x, uint32_t center_y, uint32_t radius_tiles,
                                                       uint32_t selected_id, const PluginGraphMapNode *nodes, size_t node_count,
                                                       const PluginGraphMapEdge *edges, size_t edge_count,
                                                       const PluginGraphMapObject *objects, size_t object_count);
  using FnPluginUserInterfaceUpdateGraphMapPrimitives = void (*)(uint64_t component_id, const PluginGraphMapPrimitive *primitives, size_t primitive_count);
  using FnPluginUserInterfaceRegisterGraphMapSelectHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceGraphMapSelectHandler handler, void *user_data);
  using FnPluginUserInterfaceRegisterGraphMapLinkHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceGraphMapLinkHandler handler, void *user_data);
  using FnPluginUserInterfaceRegisterGraphMapObjectLinkHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceGraphMapObjectLinkHandler handler, void *user_data);
  using FnPluginUserInterfaceRegisterGraphMapBackgroundHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceGraphMapBackgroundHandler handler, void *user_data);
  using FnPluginUserInterfaceRegisterGraphMapContextHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceGraphMapContextHandler handler, void *user_data);
  using FnPluginUserInterfaceRegisterGraphMapZoomHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceGraphMapZoomHandler handler, void *user_data);

  using FnPluginEventBusReceiver = void (*)(void *args, void *context);
  using FnPluginEventBusRegister = void (*)(const char *id, void *receiver, void *context);

  using FnPluginExposeFunction = void (*)(const char *name, FnPluginExposedFunction fn, void *context);
  using FnPluginGetExposedFunction = FnPluginExposedFunction (*)(const char *name);
  using FnPluginUiIsVisible = bool (*)();

  using FnPluginResolve = void *(*)(const char *name);

  struct PluginApi
  {
    uint32_t version;
    FnPluginResolve resolve;
  };

  struct PluginHostApi
  {
    FnPluginLog log = nullptr;
    FnPluginGetConfigurationDir get_configuration_dir = nullptr;
    FnPluginGetGlobals get_globals = nullptr;

    FnPluginUserInterfaceAllocateComponent ui_allocate_component = nullptr;
    FnPluginUserInterfaceUpdateComponentText ui_update_component_text = nullptr;
    FnPluginUserInterfaceUpdateComponentItems ui_update_component_items = nullptr;
    FnPluginUserInterfaceIsComponentActive ui_is_component_active = nullptr;
    FnPluginUserInterfaceSetActive ui_set_component_active = nullptr;
    FnPluginUserInterfaceRegisterDropDownChangeHandler ui_register_dropdown_change_handler = nullptr;
    FnPluginUserInterfaceRegisterButtonClickHandler ui_register_button_click_handler = nullptr;
    FnPluginUserInterfaceDropDownSetSelected ui_dropdown_set_selected = nullptr;
    FnPluginUserInterfaceSetVisible ui_set_visible = nullptr;
    FnPluginUserInterfaceUpdateGraphMap ui_update_graph_map = nullptr;
    FnPluginUserInterfaceUpdateGraphMapPrimitives ui_update_graph_map_primitives = nullptr;
    FnPluginUserInterfaceRegisterGraphMapSelectHandler ui_register_graph_map_select_handler = nullptr;
    FnPluginUserInterfaceRegisterGraphMapLinkHandler ui_register_graph_map_link_handler = nullptr;
    FnPluginUserInterfaceRegisterGraphMapObjectLinkHandler ui_register_graph_map_object_link_handler = nullptr;
    FnPluginUserInterfaceRegisterGraphMapBackgroundHandler ui_register_graph_map_background_handler = nullptr;
    FnPluginUserInterfaceRegisterGraphMapContextHandler ui_register_graph_map_context_handler = nullptr;
    FnPluginUserInterfaceRegisterGraphMapZoomHandler ui_register_graph_map_zoom_handler = nullptr;

    FnPluginEventBusRegister event_bus_register = nullptr;

    FnPluginExposeFunction expose_function = nullptr;
    FnPluginGetExposedFunction get_exposed_function = nullptr;
    FnPluginUiIsVisible ui_is_visible = nullptr;
  };

  struct Plugin
  {
    const char *name = nullptr;
    const char *version = nullptr;
    uint64_t ui_tab_container_id;

    FnPluginGetName get_name;
    FnPluginGetVersion get_version;
    FnPluginInit init;

    PluginApi api;
  };

  class PluginManager
  {
  private:
    std::vector<std::unique_ptr<Plugin>> plugins;
    PluginApi api;

  private:
    std::vector<std::function<void(Plugin *)>> plugin_load_callbacks;

  public:
    void init();
    void add_load_callback(std::function<void(Plugin *)> function);

  private:
    void load(const std::string &path);

  public:
    void load_all(const std::string &path);

  public:
    const std::vector<std::unique_ptr<Plugin>> &view_plugins() const;
  };
} // namespace crs
