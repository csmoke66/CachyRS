#include "developer.h"
#include "cachy.h"
#include "not_cachy.h"

#include <imgui.h>

namespace crs
{
    void DeveloperOverlay::render_player_overlay(ImDrawList *draw_list, const WorldNode *root)
    {
        // clang-format off
        iterate_entities(root, [&draw_list](const Entity *entity) 
        {
            if (entity->type == EntityType::player)
            {
                Vec2<float> screen_pos;
                if (RS.project_to_screen(entity->position, &screen_pos))
                {
                    draw_list->AddText(ImVec2(screen_pos.x, screen_pos.y), IM_COL32(255, 0, 0, 170), entity->name.c_str());
                }
            }
        });
        // clang-format on
    }

    void DeveloperOverlay::render_npc_overlay(ImDrawList *draw_list, const WorldNode *root)
    {
    }

    void DeveloperOverlay::render_object_overlay(ImDrawList *draw_list, const WorldNode *root)
    {
    }

    void DeveloperOverlay::render_ground_item_overlay(ImDrawList *draw_list, const WorldNode *root)
    {
    }

    void DeveloperOverlay::render_widget_picker(ImDrawList *draw_list, const Engine *engine, const Widget *widget, int x, int y)
    {
        auto bg = ImGui::GetBackgroundDrawList();
        auto rendered = render_widget_hook->rendered.find(widget);
        if (rendered != render_widget_hook->rendered.end() &&
            engine->time - rendered->second.time < 10)
        {
            if (widget->get_type() == WidgetType::container)
            {
                auto container = (ContainerWidget *)widget;
                for (auto c = container->children.begin; c != container->children.end; c++)
                {
                    if (auto cw = c->widget)
                    {
                        render_widget_picker(draw_list, engine, cw, container->x + x, container->y + y);
                    }
                }
            }
            else
            {
                auto a_x = rendered->second.absolute_x;
                auto a_y = rendered->second.absolute_y;

                auto min = ImVec2(a_x, a_y);
                auto max = ImVec2(min.x + widget->width, min.y + widget->height);

                auto &mp = poll_event_hook->mouse_pos;
                if (mp.x >= min.x && mp.x < max.x && mp.y >= min.y && mp.y < max.y)
                {
                    bg->AddRectFilled(min, max, IM_COL32(0, 255, 0, 20));
                }
                else
                {
                    bg->AddRectFilled(min, max, IM_COL32(255, 0, 0, 10));
                }
            }
        }
    }

    void DeveloperOverlay::render_widget_picker(ImDrawList *draw_list, const Engine *engine, const WidgetCache *widget_cache)
    {
        for (auto widget_001 = widget_cache->c.begin;
             widget_001 != widget_cache->c.end;
             widget_001++)
        {
            if (auto widget_002 = widget_001->widget_002)
            {
                for (auto widget_003 = widget_002->widgets_003.begin;
                     widget_003 != widget_002->widgets_003.end;
                     widget_003++)
                {
                    if (auto widget = widget_003->widget)
                    {
                        render_widget_picker(draw_list, engine, widget, 0, 0);
                    }
                }
            }
        }
    }

    void DeveloperOverlay::init()
    {
        render_widget_hook = RS.hook_manager->view_hook<RenderWidgetHook>("render_widget");
        poll_event_hook = RS.hook_manager->view_hook<SdlPollEventHook>("poll_event");

        initialized = true;
    }

    void DeveloperOverlay::render()
    {
        if (!initialized)
        {
            return;
        }

        auto engine = RS.get_globals()->engine;
        if (!engine)
        {
            return;
        }

        auto widget_cache = dref<const WidgetCache *>(engine, {off(Engine, widget_cache)});
        auto root_world_node = NRS.root_node();

        auto draw_list = ImGui::GetBackgroundDrawList();
        
        // TODO FIXME integrate with DOM
        if (player_overlay_on)
        {
            render_player_overlay(draw_list, root_world_node);
        }
    }
}