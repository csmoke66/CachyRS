#include "dom.h"
#include "reversed/reversed.h"

#include <cstring>
#include <memory>
#include <string>

namespace crs
{
  inline const void *object_vtable(const void *object)
  {
    const void *vtable = nullptr;
    if (object)
    {
      std::memcpy(&vtable, object, sizeof(vtable));
    }
    return vtable;
  }

  inline void add_hidden_pointer(DomNode &node, const std::string &name, const void *value)
  {
    auto v = std::make_unique<PointerDomValue>(name, value);
    v->mark_hidden();
    node.add_value(std::move(v));
  }

  inline void add_hidden_float(DomNode &node, const std::string &name, float value)
  {
    auto v = std::make_unique<FloatDomValue>(name, value);
    v->mark_hidden();
    node.add_value(std::move(v));
  }

  inline void add_hidden_int32(DomNode &node, const std::string &name, int32_t value)
  {
    auto v = std::make_unique<Int32DomValue>(name, value);
    v->mark_hidden();
    node.add_value(std::move(v));
  }

  inline void add_hidden_uint32(DomNode &node, const std::string &name, uint32_t value)
  {
    auto v = std::make_unique<UInt32DomValue>(name, value);
    v->mark_hidden();
    node.add_value(std::move(v));
  }

  inline int32_t named_animation_id(const NamedEntity *entity)
  {
    if (!entity || entity->animation_queue.begin == entity->animation_queue.end)
    {
      return -1;
    }

    return static_cast<int32_t>(*entity->animation_queue.begin);
  }

  inline void add_named_entity_values(DomNode &node, const NamedEntity *entity)
  {
    add_hidden_pointer(node, "address", entity);
    add_hidden_pointer(node, "VT address", object_vtable(entity));
    node.add_value(std::make_unique<StringDomValue>("name", entity->name.c_str()));
    add_hidden_float(node, "scene x", entity->position.x);
    add_hidden_float(node, "scene y", entity->position.y);
    add_hidden_float(node, "scene z", entity->position.z);
    add_hidden_int32(node, "tile x", static_cast<int32_t>(entity->position.x / 512.f));
    add_hidden_int32(node, "tile y", static_cast<int32_t>(entity->position.z / 512.f));
    add_hidden_int32(node, "animation id", named_animation_id(entity));
  }

  inline void sync_named_entity_values(DomNode &node, const NamedEntity *entity)
  {
    node.set_value<StringDomValue>("name", entity->name.str());
    auto moved = node.set_value<FloatDomValue>("scene x", entity->position.x);
    moved = node.set_value<FloatDomValue>("scene y", entity->position.y) || moved;
    moved = node.set_value<FloatDomValue>("scene z", entity->position.z) || moved;
    if (moved)
    {
      node.set_value<Int32DomValue>("tile x", static_cast<int32_t>(entity->position.x / 512.f));
      node.set_value<Int32DomValue>("tile y", static_cast<int32_t>(entity->position.z / 512.f));
    }

    node.set_value<Int32DomValue>("animation id", named_animation_id(entity));
  }
} // namespace crs
