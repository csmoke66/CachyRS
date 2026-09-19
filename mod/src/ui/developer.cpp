#include "developer.h"
#include "cachy.h"
#include "game_hook.h"
#include "not_cachy.h"

#include <algorithm>
#include <cstdint>
#include <imgui.h>
#include <unordered_map>
#include <unordered_set>

namespace crs
{
  void DeveloperOverlay::render_player_overlay(ImDrawList *draw_list, WorldNode *root)
  {
    iterate_entities(root, [this, &draw_list](NamedEntity *entity)
    {
      if (entity->type == EntityType::player && (!player_overlay_target || player_overlay_target == entity))
      {
        Vec2<float> screen_pos;
        if (RS.project_to_screen(entity->position, &screen_pos))
        {
          draw_list->AddText(ImVec2(screen_pos.x, screen_pos.y), IM_COL32(255, 0, 0, 170), entity->name.c_str());
        }

        if (auto parent = entity->parent)
        {
          if (RS.project_to_screen(parent->pos_a, &screen_pos))
          {
            draw_list->AddCircle(ImVec2(screen_pos.x, screen_pos.y), 3.f, IM_COL32(255, 0, 0, 170));
          }

          if (RS.project_to_screen(parent->pos_b, &screen_pos))
          {
            draw_list->AddCircle(ImVec2(screen_pos.x, screen_pos.y), 3.f, IM_COL32(0, 255, 0, 170));
          }

          if (RS.project_to_screen(parent->pos_c, &screen_pos))
          {
            draw_list->AddCircle(ImVec2(screen_pos.x, screen_pos.y), 3.f, IM_COL32(0, 0, 255, 170));
          }

          if (RS.project_to_screen(parent->pos_avg, &screen_pos))
          {
            draw_list->AddCircle(ImVec2(screen_pos.x, screen_pos.y), 3.f, IM_COL32(255, 255, 0, 170));
          }
        }
      }
    });
  }

  void DeveloperOverlay::render_npc_overlay(ImDrawList *, WorldNode *)
  {
  }

  void DeveloperOverlay::render_object_overlay(ImDrawList *, WorldNode *)
  {
  }

  void DeveloperOverlay::render_ground_item_overlay(ImDrawList *, WorldNode *)
  {
  }

  static HoveredWidgetNode build_hovered_node(
      Widget *widget,
      const std::unordered_map<Widget *, std::vector<Widget *>> &children_of)
  {
    HoveredWidgetNode node;
    node.widget = widget;
    auto it = children_of.find(widget);
    if (it != children_of.end())
    {
      node.children.reserve(it->second.size());
      for (auto *child : it->second)
      {
        node.children.push_back(build_hovered_node(child, children_of));
      }
    }
    return node;
  }

  static void draw_hovered_node(
      ImDrawList *draw_list,
      RenderWidgetHook *hook,
      const HoveredWidgetNode &node,
      bool root)
  {
    if (node.widget && hook->is_visible(node.widget))
    {
      auto it = hook->rendered().find(node.widget);
      if (it != hook->rendered().end())
      {
        const auto &snap = it->second;
        const auto min = ImVec2(static_cast<float>(snap.absolute_x), static_cast<float>(snap.absolute_y));
        const auto max = ImVec2(min.x + static_cast<float>(snap.width), min.y + static_cast<float>(snap.height));
        const bool leaf = node.children.empty();
        draw_list->AddRect(
            min,
            max,
            leaf ? IM_COL32(0, 255, 0, 22) : (root ? IM_COL32(255, 180, 0, 12) : IM_COL32(0, 180, 255, 10)),
            0.f,
            0,
            leaf ? 1.f : 0.75f);
        if (leaf)
        {
          draw_list->AddRectFilled(min, max, IM_COL32(0, 255, 0, 3));
        }
      }
    }

    for (const auto &child : node.children)
    {
      draw_hovered_node(draw_list, hook, child, false);
    }
  }

  void DeveloperOverlay::arm_widget_pick(WidgetPickMode mode)
  {
    widget_pick_armed = true;
    widget_pick_mode = mode;
  }

  void DeveloperOverlay::render_widget_picker(ImDrawList *draw_list)
  {
    hovered_widget_tree.clear();

    if (!widget_pick_armed || !render_widget_hook || !poll_event_hook)
    {
      return;
    }

    const auto &mp = poll_event_hook->mouse_pos;
    const auto mx = static_cast<int32_t>(mp.x);
    const auto my = static_cast<int32_t>(mp.y);

    std::vector<Widget *> hits;
    std::unordered_set<Widget *> hit_set;
    std::unordered_map<Widget *, Widget *> parent_of;
    std::unordered_set<Widget *> menu_option_hits;
    for (const auto &[widget_key, snap] : render_widget_hook->rendered())
    {
      // Keys only — use snapshot fields, do not dereference widget_key for hierarchy.
      auto *widget = const_cast<Widget *>(widget_key);
      if (!widget || snap.width == 0 || snap.height == 0)
      {
        continue;
      }

      const auto x0 = snap.absolute_x;
      const auto y0 = snap.absolute_y;
      const auto x1 = x0 + static_cast<int32_t>(snap.width);
      const auto y1 = y0 + static_cast<int32_t>(snap.height);
      if (mx < x0 || mx >= x1 || my < y0 || my >= y1)
      {
        continue;
      }

      hits.push_back(widget);
      hit_set.insert(widget);
      parent_of[widget] = snap.parent;
      if (snap.has_menu_options)
      {
        menu_option_hits.insert(widget);
      }
    }

    if (widget_pick_mode == WidgetPickMode::with_menu_options)
    {
      std::unordered_set<Widget *> keep = menu_option_hits;
      for (auto *widget : menu_option_hits)
      {
        auto *parent = parent_of[widget];
        while (parent && hit_set.contains(parent))
        {
          keep.insert(parent);
          parent = parent_of[parent];
        }
      }

      hits.erase(std::remove_if(hits.begin(), hits.end(), [&](Widget *w)
      {
        return !keep.contains(w);
      }),
          hits.end());
      hit_set = keep;
    }

    if (!hits.empty())
    {
      std::unordered_map<Widget *, std::vector<Widget *>> children_of;
      std::vector<Widget *> roots;
      for (auto *widget : hits)
      {
        auto *parent = parent_of[widget];
        if (parent && hit_set.contains(parent))
        {
          children_of[parent].push_back(widget);
        }
        else
        {
          roots.push_back(widget);
        }
      }

      hovered_widget_tree.reserve(roots.size());
      for (auto *root : roots)
      {
        hovered_widget_tree.push_back(build_hovered_node(root, children_of));
      }

      for (const auto &root : hovered_widget_tree)
      {
        draw_hovered_node(draw_list, render_widget_hook, root, true);
      }
    }

    auto &io = ImGui::GetIO();
    const bool ui_blocking = io.WantCaptureMouse || (RS.ui_visible && RS.ui && RS.ui->wants_input());
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ui_blocking)
    {
      picked_widget_tree = hovered_widget_tree;
      widget_pick_armed = false;
      if (RS.dom_node_hovered_widgets)
      {
        RS.dom_node_hovered_widgets->update();
      }
    }
  }

  void DeveloperOverlay::init()
  {
    render_widget_hook = RS.hook_manager->view_hook<RenderWidgetHook>("render_widget");
    poll_event_hook = RS.hook_manager->view_hook<SdlPollEventHook>("sdl_poll_event");

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

    auto draw_list = ImGui::GetBackgroundDrawList();

    if (engine->state == GameState::lobby_screen || engine->state == GameState::in_game)
    {
      render_widget_picker(draw_list);
    }

    if (engine->state == GameState::in_game)
    {
      auto root_world_node = NRS.root_node();
      if (root_world_node && player_overlay_on)
      {
        render_player_overlay(draw_list, root_world_node);
      }
    }
  }
} // namespace crs
