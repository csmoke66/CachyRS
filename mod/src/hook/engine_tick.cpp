#include "cachy.h"
#include "not_cachy.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

namespace crs
{
    void EngineTickHook::tick_ui(Engine *engine)
    {
        if (engine->state == GameState::lobby_screen || engine->state == GameState::in_game)
        {
            RS.dom_node_item_containers->update();
        }

        if (engine->state == GameState::in_game)
        {
            RS.dom_node_npcs->update();
            RS.dom_node_players->update();
            RS.dom_node_world_settings->update();
        }
    }

    void EngineTickHook::tick_imgui(Engine *engine)
    {
        auto swap_buffers_hook = RS.hook_manager->view_hook<EglSwapBuffersHook>("egl_swap_buffers");
        if (!!swap_buffers_hook && !swap_buffers_hook->is_first_run)
        {
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            RS.developer_overlay.render();

            ImGui::Render();
        }
    }

    void EngineTickHook::tick_stats()
    {
        if (RS.stats_timer.check(std::chrono::seconds(5)))
        {
            auto item_container_dom_nodes_created_recent = RS.stats.item_container_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
            auto item_container_dom_nodes_removed_recent = RS.stats.item_container_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

            auto item_dom_nodes_created_recent = RS.stats.item_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
            auto item_dom_nodes_removed_recent = RS.stats.item_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

            auto player_dom_nodes_created_recent = RS.stats.player_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
            auto player_dom_nodes_removed_recent = RS.stats.player_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

            auto npc_dom_nodes_created_recent = RS.stats.npc_dom_nodes_created_recent.exchange(0, std::memory_order_acquire);
            auto npc_dom_nodes_removed_recent = RS.stats.npc_dom_nodes_removed_recent.exchange(0, std::memory_order_acquire);

            LOG(INFO, " -> Stats");
            LOG(INFO, " -> New Item Container DOM Nodes: " << RS.stats.item_container_dom_nodes_created << "/" << item_container_dom_nodes_created_recent << "/" << item_container_dom_nodes_removed_recent);

            LOG(INFO, " -> New Item DOM Nodes: " << RS.stats.item_dom_nodes_created << "/" << item_dom_nodes_created_recent << "/" << item_dom_nodes_removed_recent);

            LOG(INFO, " -> New Player DOM Nodes: " << RS.stats.player_dom_nodes_created << "/" << player_dom_nodes_created_recent << "/" << player_dom_nodes_removed_recent);

            LOG(INFO, " -> New NPC DOM Nodes: " << RS.stats.npc_dom_nodes_created << "/" << npc_dom_nodes_created_recent << "/" << npc_dom_nodes_removed_recent);

            LOG(INFO, " -> Push UI State Time: " << RS.stats.push_ui_state_stopwatch.check_ms());
            LOG(INFO, " -> Render UI Time: " << RS.stats.render_ui_stopwatch.check_ms());
        }
    }

    void EngineTickHook::watch_item_changes(Engine *engine)
    {
        // clang-format off
        auto get_cached_container = [this](uint32_t idx) -> ItemContainerCache&
        {
            auto it = this->cached_containers.find(idx);
            if (it == this->cached_containers.end())
            {
                this->cached_containers[idx] = ItemContainerCache();
                return this->cached_containers[idx];
            }
            return it->second;
        };
        // clang-format on

        if (auto cache = engine->item_cache)
        {
            for (auto it = cache->containers.begin; it != cache->containers.end; it++)
            {
                auto& container = get_cached_container(it->id);
                if (it->items.size() > container.items.size())
                {
                    auto new_slots = it->items.size() - container.items.size();
                    auto start = container.items.size();
                    container.items.resize(it->items.size());
                }

                auto slot = 0;
                for (auto item_it = it->items.begin; item_it != it->items.end; item_it++)
                {
                    auto& container_item = container.items[slot++];
                    if (container_item.id != item_it->id || 
                        container_item.amount != item_it->amount)
                    {
                        auto delta = 0;
                        if (container_item.id == item_it->id || container_item.id == -1)
                        {
                            delta = item_it->amount - container_item.amount;
                        }

                        auto event = ItemChangedEvent(it->id, slot, container_item.id, container_item.amount, item_it->id, item_it->amount, delta);
                        RS.event_bus.dispatch(ItemChangedEvent::specific_id(), &event);

                        container_item = *item_it;
                    }
                }
            }
        }
    }

    void EngineTickHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);
        auto swap_buffers_hook = RS.hook_manager->view_hook<EglSwapBuffersHook>("egl_swap_buffers");
        if (!plugins_loaded && !swap_buffers_hook->is_first_run)
        {
            LOG(INFO, "Loading plugins...");
            RS.plugin_manager.init();
            RS.plugin_manager.load_all(RS.resolve_configuration("plugins/"));

            plugins_loaded = true;
        }

        auto engine = (Engine *)CPU_FIRST_ARG(cpu_state);

        // clang-format off
        RS.ui_locked([this, engine]()
        {
            tick_ui(engine);
            tick_imgui(engine);
            return false;
        });
        // clang-format on

        tick_stats();

        auto event = EngineTickEvent();
        RS.event_bus.dispatch(EngineTickEvent::specific_id(), &event);

        watch_item_changes(engine);

        cpu_state->rax = (uint64_t)trampoline(
            engine,
            CPU_FIRST_FARG(cpu_state));
    }
}