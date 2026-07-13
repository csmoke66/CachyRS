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
    };

    extern NotCachyRS NRS;
}