#pragma once
#include "reversed/reversed.h"

namespace crs
{
    class NotCachyRS
    {
    public:
        Scene003 *scene_003() const;
        WorldNode *root_node() const;
        WidgetCache *widget_cache() const;
        SDL_Window *sdl_window() const;
        ItemCache *item_cache() const;
        PlayerUpdateCache *player_update_cache() const;
        NpcUpdateCache *npc_update_cache() const;
        Cache001 *cache() const;
        CacheIndex *cache_index(CacheIndexOrdinal ordinal) const;
        CacheIndex *cache_index_world_settings() const;
        WorldSettingCache *world_setting_cache() const;
        uint32_t mask_world_setting(const WorldSetting *setting, const WorldSettingMask *mask) const;
    };

    extern NotCachyRS NRS;

    template <typename FN>
    static void iterate_entities(WorldNode *node, FN fn)
    {
        if (node)
        {
            if (auto entity = node->entity)
            {
                fn((NamedEntity *)entity);
            }

            for (auto c = node->children.begin; c != node->children.end; c++)
            {
                iterate_entities(*c, fn);
            }
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
                    if (auto player = update->player)
                    {
                        fn(player);
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
                    if (auto npc = p->npc)
                    {
                        fn(npc);
                    }
                }
            }
        }
    }
}