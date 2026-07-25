#include "cachy.h"
#include "not_cachy.h"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>

namespace crs
{
    void EngineTickHook::tick_ui(Engine *engine)
    {
        RS.ui_mutex.lock();
        if (engine->state == GameState::lobby_screen || engine->state == GameState::in_game)
        {
            RS.dom_node_item_containers->update();
        }

        if (engine->state == GameState::in_game)
        {
            RS.dom_node_npcs->update();
            RS.dom_node_players->update();
        }
        RS.ui_mutex.unlock();
    }

    void EngineTickHook::tick_imgui(Engine *engine)
    {
        auto swap_buffers_hook = RS.hook_manager->view_hook<EglSwapBuffersHook>("swap_buffers");
        if (!!swap_buffers_hook && !swap_buffers_hook->is_first_run)
        {
            RS.imgui_mutex.lock();
            ImGui_ImplOpenGL3_NewFrame();
            ImGui::NewFrame();
            if (ImGui::Begin("Developer"))
            {
                if (ImGui::Button("Reload UI"))
                {
                    RS.ui->reload();
                }
            }

            RS.developer_overlay.render();

            ImGui::End();
            ImGui::Render();
            RS.imgui_mutex.unlock();
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

    void EngineTickHook::handler(CpuState *cpu_state)
    {
        BaseHook::handler(cpu_state);

        auto engine = (Engine *)CPU_FIRST_ARG(cpu_state);
        tick_ui(engine);
        tick_imgui(engine);
        tick_stats();

        auto event = EngineTickEvent();
        RS.event_bus.dispatch(EngineTickEvent::specific_id(), &event);

        cpu_state->rax = (uint64_t)trampoline(
            engine,
            CPU_FIRST_FARG(cpu_state));
    }
}