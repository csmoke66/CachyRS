#include "not_cachy.h"
#include "cachy.h"

namespace crs
{
    NotCachyRS NRS;

    const Scene003 *NotCachyRS::scene_003() const
    {
        auto globals = RS.get_globals();
        auto engine = globals->engine;
        if (!engine)
        {
            return nullptr;
        }

        auto scene_001 = engine->scene_001;
        if (!scene_001)
        {
            return nullptr;
        }

        auto scene_002 = scene_001->scene_002.reference(scene_001->scene_index);
        if (!scene_002)
        {
            return nullptr;
        }

        return scene_002->scene_003;
    }

    const WorldNode *NotCachyRS::root_node() const
    {
        return dref<const WorldNode *>(scene_003(), {off(Scene003, world_root)});
    }

    const WidgetCache *NotCachyRS::widget_cache() const
    {
        return dref<const WidgetCache *>(
            RS.get_globals(),
            {off(Globals, engine),
             off(Engine, widget_cache)});
    }

    SDL_Window *NotCachyRS::sdl_window() const
    {
        return dref<SDL_Window *>(
            RS.get_globals(),
            {off(Globals, linux_001),
             off(Linux001, linux_002),
             off(Linux002, linux_003),
             off(Linux003, linux_004),
             off(Linux004, linux_005),
             off(Linux005, sdl_window)});
    }

    const ItemCache *NotCachyRS::item_cache() const
    {
        return dref<const ItemCache *>(
            RS.get_globals(),
            {off(Globals, engine),
             off(Engine, item_cache)});
    }

    const PlayerUpdateCache *NotCachyRS::player_update_cache() const
    {
        return dref<const PlayerUpdateCache *>(
            RS.get_globals(),
            {off(Globals, engine),
             off(Engine, player_update_cache)});
    }

    const NpcUpdateCache *NotCachyRS::npc_update_cache() const
    {
        return dref<const NpcUpdateCache *>(
            RS.get_globals(),
            {off(Globals, engine),
             off(Engine, npc_update_cache)});
    }

    const Cache001 *NotCachyRS::cache() const
    {
        return dref<const Cache001 *>(
            RS.get_globals(),
            {off(Globals, engine),
             off(Engine, cache)});
    }

    const CacheIndex *NotCachyRS::cache_index(CacheIndexOrdinal ordinal) const
    {
        if (auto cache = this->cache())
        {
            return cache->indices[(uint8_t)ordinal];
        }

        return nullptr;
    }

    const WorldSettingCache *NotCachyRS::world_setting_cache() const
    {
        auto engine = RS.get_globals()->engine;
        if (!engine)
        {
            return nullptr;
        }

        return &engine->world_settings;
    }

    uint32_t NotCachyRS::mask_world_setting(const WorldSetting *setting, const WorldSettingMask *mask) const
    {
        return ((1 << (mask->end + 1 - mask->begin)) - 1) & (setting->value >> mask->begin);
    }
}