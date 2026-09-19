#include "cachy.h"
#include "dom_sync.h"
#include "game_dom.h"
#include "game_hook.h"

#include <format>
#include <vector>

namespace crs
{
  HoveredWidgetDomNode::HoveredWidgetDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : DomNode(tree, id, type)
  {
  }

  HoveredWidgetsDomNode::HoveredWidgetsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<HoveredWidgetDomNode>(tree, id, type)
  {
  }

  static const RenderedWidgetSnapshot *snapshot_for(RenderWidgetHook *rw, Widget *widget)
  {
    if (!rw || !widget)
    {
      return nullptr;
    }

    auto it = rw->rendered().find(widget);
    return it != rw->rendered().end() ? &it->second : nullptr;
  }

  static void sync_menu_options(DomNode &node, Widget *widget)
  {
    const auto count = widget->menu_options.size();
    for (uint32_t i = 0; i < count; i++)
    {
      auto *opt = widget->menu_options.reference(i);
      const auto text = opt ? opt->text.str() : std::string{};
      const auto name = std::format("option {}", i + 1);
      if (auto *existing = node.find_value<StringDomValue>(name))
      {
        existing->set(text);
      }
      else
      {
        node.add_value(std::make_unique<StringDomValue>(name, text));
      }
    }
  }

  static void sync_child_path(DomNode &node, const std::vector<uint16_t> &child_path)
  {
    node.remove_value("path");

    for (size_t i = 0; i < child_path.size(); i++)
    {
      const auto name = std::format("path {}", i + 1);
      const auto id = static_cast<uint32_t>(child_path[i]);
      if (auto *existing = node.find_value<UInt32DomValue>(name))
      {
        existing->set(id);
      }
      else
      {
        add_uint32(node, name, id);
      }
    }
  }

  static void sync_hovered_widget_values(
      DomNode &node,
      Widget *widget,
      uint32_t depth,
      const std::vector<uint16_t> &child_path,
      const RenderedWidgetSnapshot *snap)
  {
    node.set_value<PointerDomValue>("address", widget);
    node.set_value<UInt32DomValue>("depth", depth);
    node.set_value<UInt32DomValue>("parent id", widget->parent_id);
    node.set_value<UInt32DomValue>("child id", widget->child_id);
    sync_child_path(node, child_path);
    node.set_value<UInt32DomValue>("type", static_cast<uint32_t>(widget->get_type()));
    node.set_value<Int32DomValue>("abs x", snap ? snap->absolute_x : 0);
    node.set_value<Int32DomValue>("abs y", snap ? snap->absolute_y : 0);
    node.set_value<UInt32DomValue>("width", snap ? snap->width : 0);
    node.set_value<UInt32DomValue>("height", snap ? snap->height : 0);
    sync_menu_options(node, widget);
  }

  static std::shared_ptr<HoveredWidgetDomNode> make_hovered_widget_node(
      const std::shared_ptr<DomTree> &tree,
      Widget *widget,
      uint32_t depth,
      const std::vector<uint16_t> &child_path,
      const RenderedWidgetSnapshot *snap)
  {
    auto node = std::make_shared<HoveredWidgetDomNode>(tree, make_pointer_id("hovered_widget_", widget), "hovered_widget");
    add_pointer(*node, "address", widget);
    add_uint32(*node, "depth", depth);
    add_uint32(*node, "parent id", widget->parent_id);
    add_uint32(*node, "child id", widget->child_id);
    sync_child_path(*node, child_path);
    add_inlined_uint32(*node, "type", static_cast<uint32_t>(widget->get_type()));
    add_int32(*node, "abs x", snap ? snap->absolute_x : 0);
    add_int32(*node, "abs y", snap ? snap->absolute_y : 0);
    add_uint32(*node, "width", snap ? snap->width : 0);
    add_uint32(*node, "height", snap ? snap->height : 0);
    sync_menu_options(*node, widget);
    return node;
  }

  static void sync_hovered_nodes(
      DomNode &parent,
      const std::shared_ptr<DomTree> &tree,
      RenderWidgetHook *rw,
      const std::vector<HoveredWidgetNode> &nodes,
      uint32_t depth,
      const std::vector<uint16_t> &path_prefix)
  {
    parent.begin_sync();

    for (const auto &node : nodes)
    {
      if (!node.widget)
      {
        continue;
      }

      // Skip widgets that are no longer in the render cache (or zero-sized).
      if (!rw || !rw->is_visible(node.widget))
      {
        continue;
      }

      auto child_path = path_prefix;
      child_path.push_back(node.widget->child_id);

      auto *snap = snapshot_for(rw, node.widget);
      auto key = reinterpret_cast<uintptr_t>(node.widget);
      if (auto existing = parent.touch_keyed(key))
      {
        sync_hovered_widget_values(*existing, node.widget, depth, child_path, snap);
        sync_hovered_nodes(*existing, tree, rw, node.children, depth + 1, child_path);
      }
      else
      {
        auto created = make_hovered_widget_node(tree, node.widget, depth, child_path, snap);
        parent.add_keyed_child(key, created);
        sync_hovered_nodes(*created, tree, rw, node.children, depth + 1, child_path);
      }
    }

    parent.end_sync();
  }

  void HoveredWidgetsDomNode::update()
  {
    auto *rw = RS.hook_manager ? RS.hook_manager->view_hook<RenderWidgetHook>("render_widget") : nullptr;
    sync_hovered_nodes(*this, tree, rw, RS.developer_overlay.picked_widget_tree, 0, {});
  }
} // namespace crs
