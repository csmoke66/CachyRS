#include "plugin.h"
#include "log.h"
#include "cachy.h"

#include <dlfcn.h>

#include <filesystem>

namespace crs
{
    class CEventBusReceiver : public EventReceiver<Event>
    {
    private:
        FnPluginEventBusReceiver receiver;
        void *context;

    public:
        CEventBusReceiver(FnPluginEventBusReceiver receiver, void *context)
        {
            this->receiver = receiver;
            this->context = context;
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
        return RS.ui->allocate_component((ComponentType)type, parent_id);
    }

    static void plugin_api_user_interface_update_component_text(uint64_t component_id, const char *text)
    {
        RS.ui->update_component_text(component_id, std::string(text));
    }

    static bool plugin_api_user_interface_is_component_checked(uint64_t component_id)
    {
        return RS.ui->is_component_checked(component_id);
    }

    static void plugin_api_event_bus_register(const char *id, FnPluginEventBusReceiver receiver, void *context)
    {
        RS.event_bus.add_receiver(std::string(id), new CEventBusReceiver(receiver, context));
    }

    void PluginManager::init()
    {
        api.log = plugin_api_log;
        api.get_globals = plugin_api_get_globals;

        api.ui_allocate_component = plugin_api_user_interface_allocate_component;
        api.ui_update_component_text = plugin_api_user_interface_update_component_text;
        api.ui_is_component_checked = plugin_api_user_interface_is_component_checked;

        api.event_bus_register = (FnPluginEventBusRegister)plugin_api_event_bus_register;
    }

    void PluginManager::add_load_callback(std::function<void(Plugin *)> function)
    {
        plugin_load_callbacks.push_back(function);
    }

#ifdef __linux__
#define REQUIRED_EXTENSION ".so"

    void PluginManager::load(const ::std::string &path)
    {
        auto handle = dlopen(path.c_str(), RTLD_NOW);
        if (!handle)
        {
            LOG(ERROR, "Failed to load plugin at '" << path << "'");
            return;
        }

        auto get_name = (FnPluginGetName)dlsym(handle, "plugin_get_name");
        if (!get_name)
        {

            LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_get_name'");
            return;
        }

        auto init = (FnPluginInit)dlsym(handle, "plugin_init");
        if (!init)
        {

            LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_init'");
            return;
        }

        auto name = get_name();
        
        auto new_plugin = std::make_unique<Plugin>();
        new_plugin->name = name;
        new_plugin->api = api;

        for (auto& function : plugin_load_callbacks)
        {
            function(new_plugin.get());
        }

        init(new_plugin.get());

        LOG(INFO, "Loaded plugin '" << new_plugin->name << "' at '" + path << "'");
        plugins.push_back(std::move(new_plugin));
    }
#else
    UNSUPPORTED_OS();
#endif

    void PluginManager::load_all(const ::std::string &path)
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
}