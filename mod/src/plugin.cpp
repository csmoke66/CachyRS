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
        void* context;

    public:
        CEventBusReceiver(FnPluginEventBusReceiver receiver, void* context)
        {
            this->receiver = receiver;
            this->context = context;
        }

        void receive(Event *event) override
        {
            receiver(event->get_args(), context);
        }
    };

    static void plugin_api_log(const char* message)
    {
        LOG(PLUGIN, message);
    }

    static ThreadOwned<Globals*> plugin_api_get_globals()
    {
        return RS.get_globals();
    }

    static void plugin_api_event_bus_register(const char* id, FnPluginEventBusReceiver receiver, void* context)
    {
        RS.event_bus.add_receiver(std::string(id), new CEventBusReceiver(receiver, context));
    }

    void PluginManager::init()
    {
        api.log = plugin_api_log;
        api.get_globals = plugin_api_get_globals;
        api.event_bus_register = (FnPluginEventBusRegister)plugin_api_event_bus_register;
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

        auto init = (FnPluginInit)dlsym(handle, "plugin_init");
        if (!init)
        {

            LOG(ERROR, "Plugin at '" << path << "' does not export 'plugin_init'");
            return;
        }

        auto new_plugin = std::make_unique<Plugin>();
        new_plugin->api = api;

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