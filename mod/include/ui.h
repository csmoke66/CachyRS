#pragma once
#include <cstdint>
#include <functional>
#include <map>
#include <vector>

#include <SDL2/SDL.h>

#include "dom.h"
#include "math.h"

namespace crs
{
  enum class WidgetPickMode
  {
    all,
    with_menu_options,
  };

  enum class ComponentType
  {
    container,
    label,
    hr,
    line,
    button,
    checkbox,
    dropdown,
    graph_map,
    row,
  };

  struct GraphMapNode
  {
    uint32_t id = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint8_t flags = 0; // bit0: has edge to another plane
  };

  struct GraphMapEdge
  {
    uint32_t from = 0;
    uint32_t to = 0;
    uint8_t kind = 0; // 0 = walk, 1 = action
  };

  struct GraphMapObject
  {
    uint32_t object_id = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t option = 0;
    std::string name;
  };

  enum class GraphMapPrimitiveKind : uint8_t
  {
    line = 0, // (x0,y0) -> (x1,y1); thickness = full width in tiles
    disc = 1, // center (x0,y0); thickness = radius in tiles
  };

  struct GraphMapPrimitive
  {
    GraphMapPrimitiveKind kind = GraphMapPrimitiveKind::line;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    float thickness = 1.f;
    float x0 = 0.f;
    float y0 = 0.f;
    float x1 = 0.f;
    float y1 = 0.f;
  };

  struct Component
  {
  public:
    uint64_t id;
    std::vector<Component *> children;

  public:
    Component(uint64_t id) : id(id)
    {
    }

  public:
    Component *get_child(size_t child_id)
    {
      if (this->id == child_id)
      {
        return this;
      }

      for (auto c : children)
      {
        if (auto cc = c->get_child(child_id))
        {
          return cc;
        }
      }

      return nullptr;
    }
  };

  class UserInterface
  {
  private:
    bool wants_verify = false;

  public:
    virtual ~UserInterface();

  public:
    virtual void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) = 0;
    virtual void reload();
    virtual void add_reload_callback(std::function<void()> function);
    virtual void request_verify();
    virtual bool check_verify();

  public:
    virtual void process(SDL_Event *event) = 0;
    virtual bool wants_input() = 0;

  public:
    virtual uint64_t allocate_tab(const std::string &name) = 0;
    virtual uint64_t allocate_component(ComponentType type, uint64_t parent_id) = 0;
    virtual bool has_component(uint64_t component_id) = 0;
    virtual bool component_is_type(uint64_t component_id, ComponentType type) = 0;
    virtual void update_component_text(uint64_t component_id, const std::string &text) = 0;
    virtual void update_component_items(uint64_t component_id, const std::vector<std::string> &items) = 0;
    virtual bool is_component_active(uint64_t component_id) = 0;
    virtual void set_component_active(uint64_t component_id, bool active) = 0;
    virtual void register_dropdown_change_handler(uint64_t component_id, std::function<void(int32_t)> handler) = 0;
    virtual void register_button_click_handler(uint64_t component_id, std::function<void()> handler) = 0;
    virtual void dropdown_set_selected(uint64_t component_id, int32_t index) = 0;
    virtual void set_component_visible(uint64_t component_id, bool visible) = 0;

    virtual void update_graph_map(uint64_t component_id, uint32_t center_x, uint32_t center_y, uint32_t radius_tiles,
                                  uint32_t selected_id, const std::vector<GraphMapNode> &nodes,
                                  const std::vector<GraphMapEdge> &edges,
                                  const std::vector<GraphMapObject> &objects) = 0;
    virtual void update_graph_map_primitives(uint64_t component_id, const std::vector<GraphMapPrimitive> &primitives) = 0;
    virtual void register_graph_map_select_handler(uint64_t component_id, std::function<void(uint32_t)> handler) = 0;
    virtual void register_graph_map_link_handler(uint64_t component_id, std::function<void(uint32_t, uint32_t)> handler) = 0;
    virtual void register_graph_map_object_link_handler(uint64_t component_id,
                                                        std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)> handler) = 0;
    virtual void register_graph_map_background_handler(uint64_t component_id, std::function<void(uint32_t, uint32_t)> handler) = 0;
    virtual void register_graph_map_context_handler(uint64_t component_id,
                                                    std::function<void(uint32_t, uint32_t, uint32_t, int32_t)> handler) = 0;
    virtual void register_graph_map_zoom_handler(uint64_t component_id, std::function<void(uint32_t)> handler) = 0;

  public:
    virtual void render() = 0;
  };
} // namespace crs