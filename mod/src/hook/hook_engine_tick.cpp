#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    void EngineTickHook::handler(CpuState *cpu_state)
    {
        auto engine = (Engine *)CPU_FIRST_ARG(cpu_state);

        RS.mutex.lock();
        if (engine->state == GameState::lobby_screen || engine->state == GameState::in_game)
        {
            RS.dom_node_item_containers->update();
        }
        if (engine->state == GameState::in_game)
        {
            RS.dom_node_npcs->update();
            RS.dom_node_players->update();
        }
        RS.mutex.unlock();

        cpu_state->rax = (uint64_t)trampoline(
            engine,
            CPU_FIRST_FARG(cpu_state));
    }
}