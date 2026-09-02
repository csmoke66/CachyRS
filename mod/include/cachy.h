#pragma once
#include "process.h"

#include "ui.h"
#include "rml_ui.h"
#include "developer.h"

#include "ownership.h"
#include "game_dom.h"
#include "reversed/reversed.h"
#include "hook.h"
#include "game_hook.h"

#include "ring_buffer.h"
#include "util.h"
#include "timer.h"
#include "log.h"
#include "interop.h"
#include "plugin.h"

#include "event_bus.h"
#include "game_events.h"

#include "version.hpp"

#include <fstream>
#include <mutex>
#include <atomic>

#include <stdarg.h>

namespace crs
{
    struct Stats
    {
        uint64_t player_dom_nodes_created = 0;
        std::atomic<uint64_t> player_dom_nodes_created_recent = 0;
        std::atomic<uint64_t> player_dom_nodes_removed_recent = 0;
        
        uint64_t npc_dom_nodes_created = 0;
        std::atomic<uint64_t> npc_dom_nodes_created_recent = 0;
        std::atomic<uint64_t> npc_dom_nodes_removed_recent = 0;

        uint64_t item_container_dom_nodes_created = 0;
        std::atomic<uint64_t> item_container_dom_nodes_created_recent = 0;
        std::atomic<uint64_t> item_container_dom_nodes_removed_recent = 0;

        uint64_t item_dom_nodes_created = 0;
        std::atomic<uint64_t> item_dom_nodes_created_recent = 0;
        std::atomic<uint64_t> item_dom_nodes_removed_recent = 0;
        
        Stopwatch push_ui_state_stopwatch;
        Stopwatch render_ui_stopwatch;
    };

    class CachyRS
    {
    public:
        Stats stats;
        Timer stats_timer;
        
    public:
        ::std::recursive_mutex ui_mutex;

    public:
        ProcessInterface pi;
        ::std::unique_ptr<HookManager> hook_manager = nullptr;
        PluginManager plugin_manager;
        
    public:
        std::shared_ptr<ItemContainersDomNode> dom_node_item_containers;
        std::shared_ptr<PlayersDomNode> dom_node_players;
        std::shared_ptr<NpcsDomNode> dom_node_npcs;
        std::shared_ptr<WorldSettingsDomNode> dom_node_world_settings;

    public:
        bool ui_visible = false;
        DeveloperOverlay developer_overlay;
        ::std::shared_ptr<UserInterface> ui = nullptr;
        ::std::shared_ptr<DomTree> dom_tree = nullptr;
        RingBuffer<SDL_Event> event_ring_buffer;

    public:
        EventBus event_bus;

    private:
        void init_process_info();
        void init_imgui();
        void init_dom();
        void init_hooks();

    public:
        ::std::string get_configuration_dir() const;
        ::std::string resolve_configuration(const ::std::string &file) const;

    public:
        ThreadOwned<Globals *> get_globals() const;

    public:
        bool project_to_screen(const Vec3<float>& scene, Vec2<float> *out) const;

    public:
        void init();

    public:
        void push_ui_state();

    public:
        template<typename T>
        auto ui_locked(T t)
        {
            ui_mutex.lock();
            auto x = t();
            ui_mutex.unlock();
            return x;
        }

        template<typename T>
        auto ui_locked_nr(T t)
        {
            ui_mutex.lock();
            t();
            ui_mutex.unlock();
        }
    };

    extern CachyRS RS;
}