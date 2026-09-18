#include "rml_ui.h"

#include <RmlUi/Core/Geometry.h>
#include <RmlUi/Core/MeshUtilities.h>
#include <RmlUi/Core/RenderManager.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace crs
{
  namespace
  {
    Rml::ColourbPremultiplied rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
      return Rml::Colourb(r, g, b, a).ToPremultiplied();
    }

    void append_line(Rml::Mesh &mesh, Rml::Vector2f a, Rml::Vector2f b, float thickness, Rml::ColourbPremultiplied color)
    {
      auto dx = b.x - a.x;
      auto dy = b.y - a.y;
      auto len = std::sqrt(dx * dx + dy * dy);
      if (len < 0.001f)
      {
        return;
      }

      auto nx = (-dy / len) * (thickness * 0.5f);
      auto ny = (dx / len) * (thickness * 0.5f);

      auto base = static_cast<int>(mesh.vertices.size());
      mesh.vertices.push_back(Rml::Vertex{ { a.x + nx, a.y + ny }, color, {} });
      mesh.vertices.push_back(Rml::Vertex{ { a.x - nx, a.y - ny }, color, {} });
      mesh.vertices.push_back(Rml::Vertex{ { b.x - nx, b.y - ny }, color, {} });
      mesh.vertices.push_back(Rml::Vertex{ { b.x + nx, b.y + ny }, color, {} });
      mesh.indices.push_back(base + 0);
      mesh.indices.push_back(base + 1);
      mesh.indices.push_back(base + 2);
      mesh.indices.push_back(base + 0);
      mesh.indices.push_back(base + 2);
      mesh.indices.push_back(base + 3);
    }

    struct MapGeom
    {
      float width = 0.f;
      float height = 0.f;
      float scale = 1.f;
    };

    MapGeom make_map_geom(const GraphMapUiState &state, float width, float height)
    {
      constexpr float pad = 6.f;
      MapGeom g{ width, height, 1.f };
      auto radius = static_cast<float>(state.radius_tiles ? state.radius_tiles : 1);
      auto inner = std::min(std::max(1.f, width - pad * 2.f), std::max(1.f, height - pad * 2.f));
      g.scale = inner / (radius * 2.f);
      return g;
    }

    Rml::Vector2f tile_to_px(const GraphMapUiState &state, float tile_x, float tile_y, const MapGeom &geom)
    {
      return {
        (tile_x - static_cast<float>(state.center_x)) * geom.scale + geom.width * 0.5f,
        (static_cast<float>(state.center_y) - tile_y) * geom.scale + geom.height * 0.5f
      };
    }

    Rml::Vector2f tile_to_px(const GraphMapUiState &state, uint32_t tile_x, uint32_t tile_y, const MapGeom &geom)
    {
      return tile_to_px(state, static_cast<float>(tile_x), static_cast<float>(tile_y), geom);
    }

    void append_disc(Rml::Mesh &mesh, Rml::Vector2f center, float radius, Rml::ColourbPremultiplied color, int segments = 18)
    {
      if (radius < 0.5f)
      {
        return;
      }

      auto base = static_cast<int>(mesh.vertices.size());
      mesh.vertices.push_back(Rml::Vertex{ center, color, {} });
      for (int i = 0; i <= segments; i++)
      {
        auto a = (static_cast<float>(i) / static_cast<float>(segments)) * 6.2831853f;
        mesh.vertices.push_back(Rml::Vertex{ { center.x + std::cos(a) * radius, center.y + std::sin(a) * radius }, color, {} });
      }
      for (int i = 0; i < segments; i++)
      {
        mesh.indices.push_back(base);
        mesh.indices.push_back(base + 1 + i);
        mesh.indices.push_back(base + 2 + i);
      }
    }
  } // namespace

  Rml::DecoratorDataHandle VisibilityTrackerDecorator::GenerateElementData(Rml::Element *, Rml::BoxArea) const
  {
    return 1;
  }

  void VisibilityTrackerDecorator::ReleaseElementData(Rml::DecoratorDataHandle) const
  {
  }

  bool VisibilityTrackerDecorator::IsElementOnScreen(Rml::Element *element, Rml::Context *context) const
  {
    if (!element || !context)
      return false;

    auto box_size = element->GetBox().GetSize(Rml::BoxArea::Border);
    if (box_size.x <= 0.0f || box_size.y <= 0.0f)
    {
      return false;
    }

    auto absolute_pos = element->GetAbsoluteOffset(Rml::BoxArea::Border);
    auto context_size = context->GetDimensions();

    bool is_outside = (absolute_pos.x + box_size.x < 0.0f) ||
                      (absolute_pos.y + box_size.y < 0.0f) ||
                      (absolute_pos.x > static_cast<float>(context_size.x)) ||
                      (absolute_pos.y > static_cast<float>(context_size.y));

    return !is_outside;
  }

  void VisibilityTrackerDecorator::RenderElement(Rml::Element *element, Rml::DecoratorDataHandle) const
  {
    if (IsElementOnScreen(element, element->GetContext()))
    {
      element->SetAttribute("render_frame", parent->get_render_frame());
    }
    else
    {
      element->SetAttribute("render_frame", 0);
    }
  }

  Rml::SharedPtr<Rml::Decorator> VisibilityTrackerInstancer::InstanceDecorator(
      const Rml::String &,
      const Rml::PropertyDictionary &,
      const Rml::DecoratorInstancerInterface &)
  {
    auto r = Rml::MakeShared<VisibilityTrackerDecorator>();
    r->parent = parent;
    return r;
  }

  Rml::DecoratorDataHandle GraphMapDecorator::GenerateElementData(Rml::Element *, Rml::BoxArea) const
  {
    return 1;
  }

  void GraphMapDecorator::ReleaseElementData(Rml::DecoratorDataHandle) const
  {
  }

  void GraphMapDecorator::RenderElement(Rml::Element *element, Rml::DecoratorDataHandle) const
  {
    if (!parent || !element)
    {
      return;
    }

    auto id_attr = element->GetId();
    if (id_attr.empty())
    {
      return;
    }

    auto component_id = static_cast<uint64_t>(std::strtoull(id_attr.c_str(), nullptr, 10));
    auto *component = parent->find_graph_map_component(component_id);
    if (!component)
    {
      return;
    }

    auto &state = component->graph_map;
    auto size_vec = element->GetBox().GetSize(Rml::BoxArea::Padding);
    if (size_vec.x <= 1.f || size_vec.y <= 1.f)
    {
      return;
    }

    auto geom = make_map_geom(state, size_vec.x, size_vec.y);

    std::unordered_map<uint32_t, Rml::Vector2f> positions;
    positions.reserve(state.nodes.size());
    for (auto &node : state.nodes)
    {
      positions[node.id] = tile_to_px(state, node.x, node.y, geom);
    }

    Rml::Mesh mesh;

    auto grid = rgba(50, 50, 58, 90);
    for (int d = -static_cast<int>(state.radius_tiles); d <= static_cast<int>(state.radius_tiles); d += 5)
    {
      auto from_x = tile_to_px(state, state.center_x + d, state.center_y - state.radius_tiles, geom);
      auto to_x = tile_to_px(state, state.center_x + d, state.center_y + state.radius_tiles, geom);
      append_line(mesh, from_x, to_x, 1.f, grid);

      auto from_y = tile_to_px(state, state.center_x - state.radius_tiles, state.center_y + d, geom);
      auto to_y = tile_to_px(state, state.center_x + state.radius_tiles, state.center_y + d, geom);
      append_line(mesh, from_y, to_y, 1.f, grid);
    }

    for (auto &prim : state.primitives)
    {
      auto color = rgba(prim.r, prim.g, prim.b, prim.a);
      if (prim.kind == GraphMapPrimitiveKind::disc)
      {
        auto center = tile_to_px(state, prim.x0, prim.y0, geom);
        append_disc(mesh, center, std::max(0.5f, prim.thickness * geom.scale), color);
      }
      else
      {
        auto a = tile_to_px(state, prim.x0, prim.y0, geom);
        auto b = tile_to_px(state, prim.x1, prim.y1, geom);
        append_line(mesh, a, b, std::max(1.f, prim.thickness * geom.scale), color);
      }
    }

    auto walk_color = rgba(120, 170, 255, 200);
    auto action_color = rgba(255, 180, 90, 220);
    for (auto &edge : state.edges)
    {
      auto from_it = positions.find(edge.from);
      auto to_it = positions.find(edge.to);
      if (from_it == positions.end() || to_it == positions.end())
      {
        continue;
      }
      append_line(mesh, from_it->second, to_it->second, edge.kind == 0 ? 1.5f : 2.5f, edge.kind == 0 ? walk_color : action_color);
    }

    if (state.dragging)
    {
      append_line(mesh, { state.drag_from_px, state.drag_from_py }, { state.drag_to_px, state.drag_to_py }, 2.f, rgba(255, 255, 120, 230));
    }

    auto player = tile_to_px(state, state.center_x, state.center_y, geom);
    Rml::MeshUtilities::GenerateQuad(mesh, { player.x - 4.f, player.y - 4.f }, { 8.f, 8.f }, rgba(80, 220, 120, 255));

    for (auto &node : state.nodes)
    {
      auto pos = positions[node.id];
      auto selected = node.id == state.selected_id;
      auto cross_plane = (node.flags & 1u) != 0;
      Rml::ColourbPremultiplied color;
      if (selected)
      {
        color = rgba(255, 230, 120, 255);
      }
      else if (cross_plane)
      {
        color = rgba(70, 210, 230, 255);
      }
      else
      {
        color = rgba(200, 200, 220, 255);
      }
      auto node_radius = selected ? 6.f : 5.f;
      Rml::MeshUtilities::GenerateQuad(mesh, { pos.x - node_radius, pos.y - node_radius }, { node_radius * 2.f, node_radius * 2.f }, color);
    }

    for (size_t i = 0; i < state.objects.size(); i++)
    {
      auto &obj = state.objects[i];
      auto pos = tile_to_px(state, obj.x, obj.y, geom);
      auto hovered = static_cast<int32_t>(i) == state.hovered_object;
      auto color = hovered ? rgba(255, 140, 60, 255) : rgba(255, 100, 40, 210);
      auto r = hovered ? 5.f : 4.f;
      Rml::MeshUtilities::GenerateQuad(mesh, { pos.x - r, pos.y - r }, { r * 2.f, r * 2.f }, color);
    }

    if (!mesh)
    {
      return;
    }

    auto *render_manager = element->GetRenderManager();
    if (!render_manager)
    {
      return;
    }

    auto geometry = render_manager->MakeGeometry(std::move(mesh));
    geometry.Render(element->GetAbsoluteOffset(Rml::BoxArea::Border));
  }

  Rml::SharedPtr<Rml::Decorator> GraphMapDecoratorInstancer::InstanceDecorator(
      const Rml::String &,
      const Rml::PropertyDictionary &,
      const Rml::DecoratorInstancerInterface &)
  {
    auto r = Rml::MakeShared<GraphMapDecorator>();
    r->parent = parent;
    return r;
  }
} // namespace crs
