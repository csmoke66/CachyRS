#pragma once
#include "reversed/reversed.h"

#include <imgui.h>
#include <vector>

#include "ui.h"

namespace crs
{
  class RenderWidgetHook;
  class SdlPollEventHook;

  struct HoveredWidgetNode
  {
    Widget *widget = nullptr;
    std::vector<HoveredWidgetNode> children;
  };

  class DeveloperOverlay
  {
  private:
    bool initialized = false;
    bool widget_pick_armed = false;
    WidgetPickMode widget_pick_mode = WidgetPickMode::all;

  public:
    bool player_overlay_on = false;
    const Entity *player_overlay_target = nullptr;
    std::vector<HoveredWidgetNode> hovered_widget_tree;
    std::vector<HoveredWidgetNode> picked_widget_tree;

  private:
    RenderWidgetHook *render_widget_hook = nullptr;
    SdlPollEventHook *poll_event_hook = nullptr;

  private:
    void render_player_overlay(ImDrawList *draw_list, WorldNode *root);
    void render_npc_overlay(ImDrawList *draw_list, WorldNode *root);
    void render_object_overlay(ImDrawList *draw_list, WorldNode *root);
    void render_ground_item_overlay(ImDrawList *draw_list, WorldNode *root);

  private:
    void render_widget_picker(ImDrawList *draw_list);

  public:
    void init();
    void arm_widget_pick(WidgetPickMode mode = WidgetPickMode::all);
    bool is_widget_pick_armed() const
    {
      return widget_pick_armed;
    }

  public:
    void render();
  };
} // namespace crs
