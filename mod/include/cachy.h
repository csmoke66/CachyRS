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
#include "log.h"
#include "interop.h"

#include "event_bus.h"

#include "version.hpp"

#include <fstream>
#include <mutex>
#include <atomic>

#include <stdarg.h>

namespace crs
{
    class CachyRS
    {
    public:
        ::std::mutex mutex;

    public:
        ProcessInterface pi;
        ::std::unique_ptr<HookManager> hook_manager = nullptr;

    public:
        std::shared_ptr<ItemContainersDomNode> dom_node_item_containers;
        std::shared_ptr<PlayersDomNode> dom_node_players;
        std::shared_ptr<NpcsDomNode> dom_node_npcs;

    public:
        DeveloperOverlay developer_overlay;
        ::std::shared_ptr<UserInterface> ui = nullptr;
        ::std::shared_ptr<DomTree> dom_tree = nullptr;
        RingBuffer<SDL_Event> event_ring_buffer;

    public:
        EventBus event_bus;

    private:
        void init_logging();
        void init_process_info();
        void init_imgui();
        void init_dom();
        void init_hooks();

    public:
        ::std::string get_configuration_dir() const;
        ::std::string resolve_configuration(const ::std::string &file) const;

    public:
        ThreadOwned<Globals*> get_globals() const;

    public:
        bool project_to_screen(const Vec3<float> scene, Vec2<float> *out) const;

    public:
        void init();

    public:
        void push_ui_state();
    };

    extern CachyRS RS;
}