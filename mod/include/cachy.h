#pragma once
#include "process.h"

#include "developer.h"
#include "rml_ui.h"
#include "ui.h"

#include "game_dom.h"
#include "game_hook.h"
#include "hook.h"
#include "ownership.h"
#include "reversed/reversed.h"

#include "interop.h"
#include "log.h"
#include "plugin.h"
#include "ring_buffer.h"
#include "timer.h"
#include "util.h"

#include "event_bus.h"
#include "game_events.h"

#include "version.hpp"

#include <atomic>
#include <fstream>
#include <mutex>

#include <cstdarg>

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
    std::mutex ui_mutex;

  public:
    ProcessInterface pi;
    std::unique_ptr<HookManager> hook_manager = nullptr;
    PluginManager plugin_manager;

  public:
    std::shared_ptr<StatsDomNode> dom_node_stats;
    std::shared_ptr<ItemContainersDomNode> dom_node_item_containers;
    std::shared_ptr<PlayersDomNode> dom_node_players;
    std::shared_ptr<NpcsDomNode> dom_node_npcs;
    std::shared_ptr<WorldSettingsDomNode> dom_node_world_settings;

  public:
    std::atomic<bool> ui_visible{ false };
    DeveloperOverlay developer_overlay;
    std::shared_ptr<UserInterface> ui = nullptr;
    std::shared_ptr<DomTree> dom_tree = nullptr;
    RingBuffer<SDL_Event> event_ring_buffer;

  public:
    EventBus event_bus;

  public:
    bool no_graphics;

  private:
    void init_process_info();
    void init_imgui();
    void init_dom();
    void init_hooks();

  public:
    std::string get_configuration_dir() const;
    std::string resolve_configuration(const std::string &file) const;

  public:
    ThreadOwned<Globals *> get_globals() const;

  public:
    bool project_to_screen(const Vec3<float> &scene, Vec2<float> *out) const;

  public:
    void init(bool disable_graphics = false);

  public:
    void push_ui_state();

  public:
    // Overlay UI is mutated from the engine thread (PluginApi in plugin.cpp) and
    // drawn on the render thread. The mutex is only for that shared render state.
    // Re-entrancy is a thread_local depth count so Rml callbacks that re-enter
    // PluginApi while swap-buffers already holds the lock stay cheap.
    inline static thread_local int ui_lock_depth = 0;

    struct UiLockDepth
    {
      UiLockDepth()
      {
        ++CachyRS::ui_lock_depth;
      }

      ~UiLockDepth()
      {
        --CachyRS::ui_lock_depth;
      }
    };

    template <typename T>
    auto ui_locked(T fn)
    {
      if (CachyRS::ui_lock_depth > 0)
      {
        UiLockDepth depth;
        return fn();
      }

      std::lock_guard lock(ui_mutex);
      UiLockDepth depth;
      return fn();
    }

    template <typename T>
    auto ui_locked_nr(T fn)
    {
      if (CachyRS::ui_lock_depth > 0)
      {
        UiLockDepth depth;
        fn();
        return;
      }

      std::lock_guard lock(ui_mutex);
      UiLockDepth depth;
      fn();
    }
  };

  extern CachyRS RS;
} // namespace crs