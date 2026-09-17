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
  };

  enum class InitType
  {
    loaded,
    refreshed
  };

  struct Plugin;

  using FnPluginGetName = const char *(*)();
  using FnPluginInit = void (*)(InitType type, Plugin *plugin);
  using FnPluginLog = void (*)(const char *message);
  using FnPluginExposedFunction = void (*)(void *context);

  using FnPluginUserInterfaceDropDownChangeHandler = void (*)(uint64_t id, int32_t selected, void *user_data);

  using FnPluginGetGlobals = ThreadOwned<Globals *> (*)();

  using FnPluginUserInterfaceAllocateComponent = uint64_t (*)(PluginComponentType type, uint64_t parent_id);
  using FnPluginUserInterfaceUpdateComponentText = void (*)(uint64_t component_id, const char *text);
  using FnPluginUserInterfaceUpdateComponentItems = void (*)(uint64_t component_id, const char **items, size_t item_count);
  using FnPluginUserInterfaceIsComponentActive = bool (*)(uint64_t component_id);
  using FnPluginUserInterfaceRegisterDropDownChangeHandler = void (*)(uint64_t component_id, FnPluginUserInterfaceDropDownChangeHandler handler, void *user_data);
  using FnPluginUserInterfaceDropDownSetSelected = void (*)(uint64_t component_id, int32_t index);
  using FnPluginUserInterfaceSetVisible = void (*)(uint64_t component_id, bool visible);
  using FnPluginUserInterfaceSetActive = void (*)(uint64_t component_id, bool active);

  using FnPluginEventBusReceiver = void (*)(void *args, void *context);
  using FnPluginEventBusRegister = void (*)(const char *id, void *receiver, void *context);

  using FnPluginExposeFunction = void (*)(const char *name, FnPluginExposedFunction fn, void *context);
  using FnPluginGetExposedFunction = FnPluginExposedFunction (*)(const char *name);

  struct PluginApi
  {
    FnPluginLog log;
    FnPluginGetGlobals get_globals;

    FnPluginUserInterfaceAllocateComponent ui_allocate_component;
    FnPluginUserInterfaceUpdateComponentText ui_update_component_text;
    FnPluginUserInterfaceUpdateComponentItems ui_update_component_items;
    FnPluginUserInterfaceIsComponentActive ui_is_component_active;
    FnPluginUserInterfaceSetActive ui_set_component_active;
    FnPluginUserInterfaceRegisterDropDownChangeHandler ui_register_dropdown_change_handler;
    FnPluginUserInterfaceDropDownSetSelected ui_dropdown_set_selected;
    FnPluginUserInterfaceSetVisible ui_set_visible;

    FnPluginEventBusRegister event_bus_register;

    FnPluginExposeFunction expose_function;
    FnPluginGetExposedFunction get_exposed_function;
  };

  struct Plugin
  {
    const char *name = nullptr;
    uint64_t ui_tab_container_id;

    FnPluginGetName get_name;
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