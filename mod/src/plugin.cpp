#include "plugin.h"
#include "cachy.h"
#include "log.h"

#include <dlfcn.h>

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace crs
{
  static std::unordered_map<std::string, FnPluginExposedFunction> exposed_functions;

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

  static void plugin_api_log(const char *message)
  {
    LOG(PLUGIN, message);
  }

  static ThreadOwned<Globals *> plugin_api_get_globals()
  {
    return RS.get_globals();
  }

  static uint64_t plugin_api_user_interface_allocate_component(PluginComponentType type, uint64_t parent_id)
  {
    return RS.ui_locked([type, parent_id]()
    {
      return RS.ui->allocate_component(static_cast<ComponentType>(type), parent_id);
    });
  }

  static void plugin_api_user_interface_update_component_text(uint64_t component_id, const char *text)
  {
    std::string copy(text ? text : "");
    RS.ui_locked_nr([component_id, &copy]()
    {
      RS.ui->update_component_text(component_id, copy);
    });
  }

  static void plugin_api_user_interface_update_component_items(uint64_t component_id, const char **items, size_t item_count)
  {
    std::vector<std::string> converted;
    converted.reserve(item_count);
    for (size_t i = 0; i < item_count; i++)
    {
      converted.push_back(items[i]);
    }

    RS.ui_locked_nr([component_id, &converted]()
    {
      RS.ui->update_component_items(component_id, converted);
    });
  }

  static bool plugin_api_user_interface_is_component_active(uint64_t component_id)
  {
    return RS.ui_locked([component_id]()
    {
      return RS.ui->is_component_active(component_id);
    });
  }

  static void plugin_api_user_interface_set_component_active(uint64_t component_id, bool active)
  {
    return RS.ui_locked_nr([component_id, active]()
    {
      RS.ui->set_component_active(component_id, active);
    });
  }

  static void plugin_api_user_interface_register_dropdown_change_handler(uint64_t component_id, FnPluginUserInterfaceDropDownChangeHandler handler, void *user_data)
  {
    RS.ui_locked_nr([component_id, handler, user_data]()
    {
      RS.ui->register_dropdown_change_handler(component_id, [component_id, handler, user_data](int index)
      {
        handler(component_id, index, user_data);
      });
    });
  }

  static void plugin_api_user_interface_dropdown_set_selected(uint64_t component_id, int32_t index)
  {
    RS.ui_locked_nr([component_id, index]()
    {
      RS.ui->dropdown_set_selected(component_id, index);
    });
  }

  static void plugin_api_user_interface_set_visible(uint64_t component_id, bool visible)
  {
    RS.ui_locked_nr([component_id, visible]()
    {
      RS.ui->set_component_visible(component_id, visible);
    });
  }

  static void plugin_api_event_bus_register(const char *id, FnPluginEventBusReceiver receiver, void *context)
  {
    RS.event_bus.add_receiver(id, new CEventBusReceiver(receiver, context));
  }

  static void plugin_api_expose_function(const char *name, FnPluginExposedFunction fn, void *context)
  {
    (void)context;
    exposed_functions[name] = fn;
  }

  static FnPluginExposedFunction plugin_api_get_exposed_function(const char *name)
  {
    auto it = exposed_functions.find(name);
    return it == exposed_functions.end() ? nullptr : it->second;
  }

  void PluginManager::init()
  {
    api.log = plugin_api_log;
    api.get_globals = plugin_api_get_globals;

    api.ui_allocate_component = plugin_api_user_interface_allocate_component;
    api.ui_update_component_text = plugin_api_user_interface_update_component_text;
    api.ui_update_component_items = plugin_api_user_interface_update_component_items;
    api.ui_is_component_active = plugin_api_user_interface_is_component_active;
    api.ui_set_component_active = plugin_api_user_interface_set_component_active;
    api.ui_register_dropdown_change_handler = plugin_api_user_interface_register_dropdown_change_handler;
    api.ui_dropdown_set_selected = plugin_api_user_interface_dropdown_set_selected;
    api.ui_set_visible = plugin_api_user_interface_set_visible;

    api.event_bus_register = reinterpret_cast<FnPluginEventBusRegister>(plugin_api_event_bus_register);

    api.expose_function = plugin_api_expose_function;
    api.get_exposed_function = plugin_api_get_exposed_function;
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

    auto get_name = reinterpret_cast<FnPluginGetName>(dlsym(handle, "plugin_get_name"));
    if (!get_name)
    {
      LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_get_name'");
      return;
    }

    auto init = reinterpret_cast<FnPluginInit>(dlsym(handle, "plugin_init"));
    if (!init)
    {
      LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_init'");
      return;
    }

    auto name = get_name();

    auto new_plugin = std::make_unique<Plugin>();
    new_plugin->name = name;
    new_plugin->get_name = get_name;
    new_plugin->init = init;
    new_plugin->api = api;

    for (auto &function : plugin_load_callbacks)
    {
      function(new_plugin.get());
    }

    init(InitType::loaded, new_plugin.get());

    LOG(INFO, "Loaded plugin '" << new_plugin->name << "' at '" + path << "'");
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