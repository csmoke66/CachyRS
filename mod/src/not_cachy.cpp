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
}