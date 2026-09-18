#include "dom.h"
#include "reversed/reversed.h"

#include <cstring>
#include <format>
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
    if (!entity || entity->animation_queue.empty())
    {
      return -1;
    }

    return static_cast<int32_t>(*entity->animation_queue.front());
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

  inline const char *object_cache_name(const ObjectCacheDesc *body)
  {
    (void)body;
    return "";
  }

  inline const char *object_cache_option(const ObjectCacheDesc *body, uint32_t index)
  {
    (void)body;
    (void)index;
    return "";
  }

  inline void sync_object_options(DomNode &node, const ObjectCacheDesc *body)
  {
    (void)node;
    (void)body;
  }

  inline void add_object_cache_values(DomNode &node, const CacheBuffer<void, ObjectCacheDesc> &cache)
  {
    add_hidden_pointer(node, "cache tag", cache.tag);
    add_hidden_pointer(node, "cache body", cache.body);
    add_hidden_uint32(node, "id", cache.body ? cache.body->id : 0);
  }

  inline void sync_object_cache_values(DomNode &node, const CacheBuffer<void, ObjectCacheDesc> &cache)
  {
    node.set_value<PointerDomValue>("cache tag", cache.tag);
    node.set_value<PointerDomValue>("cache body", cache.body);
    node.set_value<UInt32DomValue>("id", cache.body ? cache.body->id : 0);
  }

  inline void add_obj1_values(DomNode &node, const Obj1 *obj)
  {
    add_hidden_pointer(node, "address", obj);
    add_hidden_uint32(node, "type", static_cast<uint32_t>(obj->type));
    add_hidden_uint32(node, "id_1", obj->id_1);
    add_hidden_uint32(node, "id_2", obj->id_2);
    add_hidden_uint32(node, "tile x", obj->tile_position.x);
    add_hidden_uint32(node, "tile y", obj->tile_position.y);
    add_object_cache_values(node, obj->cache_buffer);
  }

  inline void sync_obj1_values(DomNode &node, const Obj1 *obj)
  {
    node.set_value<UInt32DomValue>("id_1", obj->id_1);
    node.set_value<UInt32DomValue>("id_2", obj->id_2);
    node.set_value<UInt32DomValue>("tile x", obj->tile_position.x);
    node.set_value<UInt32DomValue>("tile y", obj->tile_position.y);
    sync_object_cache_values(node, obj->cache_buffer);
  }

  inline void add_obj2_values(DomNode &node, const Obj2 *obj)
  {
    add_hidden_pointer(node, "address", obj);
    add_hidden_uint32(node, "type", static_cast<uint32_t>(obj->type));
    add_hidden_uint32(node, "tile x", obj->tile_position.x);
    add_hidden_uint32(node, "tile y", obj->tile_position.y);
    add_hidden_uint32(node, "tile 2 x", obj->tile_position_2.x);
    add_hidden_uint32(node, "tile 2 y", obj->tile_position_2.y);
    add_object_cache_values(node, obj->cache_buffer);
  }

  inline void sync_obj2_values(DomNode &node, const Obj2 *obj)
  {
    node.set_value<UInt32DomValue>("tile x", obj->tile_position.x);
    node.set_value<UInt32DomValue>("tile y", obj->tile_position.y);
    node.set_value<UInt32DomValue>("tile 2 x", obj->tile_position_2.x);
    node.set_value<UInt32DomValue>("tile 2 y", obj->tile_position_2.y);
    sync_object_cache_values(node, obj->cache_buffer);
  }
} // namespace crs
