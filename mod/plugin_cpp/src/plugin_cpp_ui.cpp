#include "plugin_cpp.h"

#include <cstdio>
#include <cstring>
#include <iterator>

namespace crs
{
  ApiComponent::ApiComponent(PluginHostApi api, uint64_t id) : api(api),
                                                           id(id)
  {
  }

  ApiContainer::ApiContainer() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiContainer::ApiContainer(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiContainer ApiContainer::add_container()
  {
    return Api::add_container(id);
  }

  ApiContainer ApiContainer::add_row()
  {
    return Api::add_row(id);
  }

  ApiLabel ApiContainer::add_label(const std::string &text)
  {
    return Api::add_label(id, text);
  }

  ApiHr ApiContainer::add_hr()
  {
    return Api::add_hr(id);
  }

  ApiCheckBox ApiContainer::add_checkbox(const std::string &text)
  {
    return Api::add_checkbox(id, text);
  }

  std::shared_ptr<ApiDropDown> ApiContainer::add_dropdown(std::vector<std::string> options)
  {
    return Api::add_dropdown(id, options);
  }

  std::shared_ptr<ApiButton> ApiContainer::add_button(const std::string &text)
  {
    return Api::add_button(id, text);
  }

  std::shared_ptr<ApiGraphMap> ApiContainer::add_graph_map()
  {
    return Api::add_graph_map(id);
  }

  ApiLabel::ApiLabel() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiLabel::ApiLabel(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  void ApiLabel::set_text(const std::string &text)
  {
    api.ui_update_component_text(id, text.c_str());
  }

  ApiHr::ApiHr(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  ApiCheckBox::ApiCheckBox() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiCheckBox::ApiCheckBox(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  bool ApiCheckBox::is_checked() const
  {
    return api.ui_is_component_active(id);
  }

  void ApiCheckBox::set_checked(bool checked)
  {
    api.ui_set_component_active(id, checked);
  }

  ApiDropDown::ApiDropDown() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiDropDown::ApiDropDown(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  void ApiDropDown::fire_changed(int32_t idx)
  {
    selected = idx;
    // Copied: a handler may register another handler or drop this component.
    auto handlers = change_handlers;
    for (auto &handler : handlers)
    {
      handler(idx);
    }
  }

  void ApiDropDown::on_changed(std::function<void(int32_t)> handler)
  {
    change_handlers.push_back(std::move(handler));
  }

  int32_t ApiDropDown::get_selected() const
  {
    return selected;
  }

  bool ApiDropDown::is_selected(int32_t index) const
  {
    return selected == index;
  }

  void ApiDropDown::set_selected(int32_t index)
  {
    selected = index;
    api.ui_dropdown_set_selected(id, index);
  }

  void ApiDropDown::set_items(const std::vector<std::string> &options)
  {
    std::vector<const char *> converted;
    converted.reserve(options.size());
    for (auto &s : options)
    {
      converted.push_back(s.c_str());
    }

    api.ui_update_component_items(id, converted.data(), converted.size());
    if (selected >= static_cast<int32_t>(options.size()))
    {
      set_selected(options.empty() ? 0 : 0);
    }
  }

  ApiButton::ApiButton() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiButton::ApiButton(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  void ApiButton::set_text(const std::string &text)
  {
    api.ui_update_component_text(id, text.c_str());
  }

  void ApiButton::fire_clicked()
  {
    auto handlers = click_handlers;
    for (auto &handler : handlers)
    {
      handler();
    }
  }

  void ApiButton::on_click(std::function<void()> handler)
  {
    click_handlers.push_back(std::move(handler));
  }

  ApiGraphMap::ApiGraphMap() : ApiComponent({}, static_cast<uint64_t>(-1))
  {
  }

  ApiGraphMap::ApiGraphMap(PluginHostApi api, uint64_t id) : ApiComponent(api, id)
  {
  }

  void ApiGraphMap::set_view(uint32_t center_x, uint32_t center_y, uint32_t radius_tiles,
                             uint32_t selected_id, const std::vector<GraphMapNodeView> &nodes,
                             const std::vector<GraphMapEdgeView> &edges,
                             const std::vector<GraphMapObjectView> &objects)
  {
    std::vector<PluginGraphMapNode> converted_nodes;
    converted_nodes.reserve(nodes.size());
    for (auto &node : nodes)
    {
      converted_nodes.push_back(PluginGraphMapNode{ node.id, node.x, node.y, node.flags, {} });
    }

    std::vector<PluginGraphMapEdge> converted_edges;
    converted_edges.reserve(edges.size());
    for (auto &edge : edges)
    {
      converted_edges.push_back(PluginGraphMapEdge{ edge.from, edge.to, edge.kind, {} });
    }

    std::vector<PluginGraphMapObject> converted_objects;
    converted_objects.reserve(objects.size());
    for (auto &obj : objects)
    {
      PluginGraphMapObject out{};
      out.object_id = obj.object_id;
      out.x = obj.x;
      out.y = obj.y;
      out.option = obj.option;
      std::snprintf(out.name, sizeof(out.name), "%s", obj.name.c_str());
      converted_objects.push_back(out);
    }

    api.ui_update_graph_map(id, center_x, center_y, radius_tiles, selected_id,
                            converted_nodes.data(), converted_nodes.size(),
                            converted_edges.data(), converted_edges.size(),
                            converted_objects.data(), converted_objects.size());
  }

  void ApiGraphMap::set_primitives(const std::vector<GraphMapPrimitiveView> &primitives)
  {
    if (!api.ui_update_graph_map_primitives)
    {
      return;
    }

    std::vector<PluginGraphMapPrimitive> converted;
    converted.reserve(primitives.size());
    for (auto &prim : primitives)
    {
      PluginGraphMapPrimitive out{};
      out.kind = static_cast<uint8_t>(prim.kind);
      out.r = prim.r;
      out.g = prim.g;
      out.b = prim.b;
      out.a = prim.a;
      out.thickness = prim.thickness;
      out.x0 = prim.x0;
      out.y0 = prim.y0;
      out.x1 = prim.x1;
      out.y1 = prim.y1;
      converted.push_back(out);
    }

    api.ui_update_graph_map_primitives(id, converted.data(), converted.size());
  }

  void ApiGraphMap::fire_select(uint32_t node_id)
  {
    auto handlers = select_handlers;
    for (auto &handler : handlers)
    {
      handler(node_id);
    }
  }

  void ApiGraphMap::fire_link(uint32_t from, uint32_t to)
  {
    auto handlers = link_handlers;
    for (auto &handler : handlers)
    {
      handler(from, to);
    }
  }

  void ApiGraphMap::fire_object_link(uint32_t from_vertex, uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option)
  {
    auto handlers = object_link_handlers;
    for (auto &handler : handlers)
    {
      handler(from_vertex, object_id, tile_x, tile_y, option);
    }
  }

  void ApiGraphMap::fire_background(uint32_t tile_x, uint32_t tile_y)
  {
    auto handlers = background_handlers;
    for (auto &handler : handlers)
    {
      handler(tile_x, tile_y);
    }
  }

  void ApiGraphMap::fire_context(uint32_t action, uint32_t tile_x, uint32_t tile_y, int32_t vertex_id)
  {
    auto handlers = context_handlers;
    for (auto &handler : handlers)
    {
      handler(action, tile_x, tile_y, vertex_id);
    }
  }

  void ApiGraphMap::fire_zoom(uint32_t radius_tiles)
  {
    auto handlers = zoom_handlers;
    for (auto &handler : handlers)
    {
      handler(radius_tiles);
    }
  }

  void ApiGraphMap::on_select(std::function<void(uint32_t)> handler)
  {
    select_handlers.push_back(std::move(handler));
  }

  void ApiGraphMap::on_link(std::function<void(uint32_t, uint32_t)> handler)
  {
    link_handlers.push_back(std::move(handler));
  }

  void ApiGraphMap::on_object_link(std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)> handler)
  {
    object_link_handlers.push_back(std::move(handler));
  }

  void ApiGraphMap::on_background(std::function<void(uint32_t, uint32_t)> handler)
  {
    background_handlers.push_back(std::move(handler));
  }

  void ApiGraphMap::on_context(std::function<void(uint32_t, uint32_t, uint32_t, int32_t)> handler)
  {
    context_handlers.push_back(std::move(handler));
  }

  void ApiGraphMap::on_zoom(std::function<void(uint32_t)> handler)
  {
    zoom_handlers.push_back(std::move(handler));
  }

  int32_t DropDownContentChanger::get_selected() const
  {
    if (!dropdown)
    {
      return 0;
    }

    return dropdown->get_selected();
  }

  bool DropDownContentChanger::is_selected(int32_t idx) const
  {
    return dropdown && dropdown->is_selected(idx);
  }

  void DropDownContentChanger::on_changed(int32_t idx)
  {
    if (idx < 0 || static_cast<size_t>(idx) >= containers.size())
    {
      return;
    }

    if (last_visible)
    {
      last_visible->set_visible(false);
      last_visible = nullptr;
    }

    last_visible = containers[static_cast<size_t>(idx)];
    if (last_visible)
    {
      last_visible->set_visible(true);
    }
  }

  void DropDownContentChanger::reset()
  {
    for (auto container : containers)
    {
      if (container)
      {
        container->set_visible(false);
      }
    }

    last_visible = nullptr;
    if (!containers.empty() && containers[0])
    {
      containers[0]->set_visible(true);
      last_visible = containers[0];
    }
  }

  std::unique_ptr<DropDownContentChanger> DropDownContentChanger::new_changer(
      uint64_t parent,
      std::vector<DropDownContentOption> options)
  {
    auto pages = std::make_unique<DropDownContentChanger>();
    std::vector<std::string> names;
    names.reserve(options.size());
    pages->containers.reserve(options.size());
    for (auto &option : options)
    {
      names.push_back(option.name);
      pages->containers.push_back(option.container);
    }

    pages->self = std::make_shared<DropDownContentChanger *>(pages.get());
    std::weak_ptr<DropDownContentChanger *> weak = pages->self;

    pages->dropdown = Api::add_dropdown(parent, names);
    pages->dropdown->on_changed([weak](int32_t idx)
    {
      if (auto alive = weak.lock())
      {
        (*alive)->on_changed(idx);
      }
    });

    return pages;
  }
} // namespace crs