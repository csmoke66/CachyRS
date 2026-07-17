#pragma once
#include "reversed/reversed.h"

namespace crs
{
    class NotCachyRS
    {
    public:
        const Scene003 *scene_003() const;
        const WorldNode *root_node() const;
        const WidgetCache *widget_cache() const;
        SDL_Window *sdl_window() const;
        const ItemCache *item_cache() const;
        const PlayerUpdateCache *player_update_cache() const;
        const NpcUpdateCache *npc_update_cache() const;
    };

    extern NotCachyRS NRS;

    template <typename FN>
    static void iterate_entities(const WorldNode *node, FN fn)
    {
        if (auto entity = node->entity)
        {
            fn((NamedEntity*)entity);
        }

        for (auto c = node->children.begin; c != node->children.end; c++)
        {
            iterate_entities(*c, fn);
        }
    }

    template <typename FN>
    static void iterate_players_update(FN fn)
    {
        if (auto update_cache = NRS.player_update_cache())
        {
            for (auto i = update_cache->updates.begin; i != update_cache->updates.end; i++)
            {
                if (auto update = *(i))
                {
                    if (auto entity = update->entity)
                    {
                        fn(entity);
                    }
                }
            }
        }
    }

    template <typename FN>
    static void iterate_npcs_update(FN fn)
    {
        if (auto update_cache = NRS.npc_update_cache())
        {
            for (auto i = 0; i < update_cache->size; i++)
            {
                if (auto p = update_cache->npcs[i])
                {
                    if (auto entity = p->entity)
                    {
                        fn(entity);
                    }
                }
            }
        }
    }
}