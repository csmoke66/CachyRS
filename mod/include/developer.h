#pragma once
#include "reversed/reversed.h"

#include <imgui.h>

namespace crs
{
    class RenderWidgetHook;
    class SdlPollEventHook;

    class DeveloperOverlay
    {
    private:
        bool initialized = false;

    public:
        bool player_overlay_on = false;

    private:
        const RenderWidgetHook *render_widget_hook;
        const SdlPollEventHook *poll_event_hook;

    private:
        void render_player_overlay(ImDrawList *draw_list, const WorldNode *root);
        void render_npc_overlay(ImDrawList *draw_list, const WorldNode *root);
        void render_object_overlay(ImDrawList *draw_list, const WorldNode *root);
        void render_ground_item_overlay(ImDrawList *draw_list, const WorldNode *root);

    private:
        void render_widget_picker(ImDrawList *draw_list, const Engine *engine, const Widget *widget, int x, int y);
        void render_widget_picker(ImDrawList *draw_list, const Engine *engine, const WidgetCache *widget_cache);

    public:
        void init();

    public:
        void render();
    };
}