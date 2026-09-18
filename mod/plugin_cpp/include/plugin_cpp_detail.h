#pragma once

#include "plugin_cpp.h"

#include <optional>
#include <utility>

namespace crs::detail
{
  constexpr int max_world_node_depth = 128;

  template <typename Fn>
  bool for_each_entity(WorldNode *node, std::optional<EntityType> type, Fn &&fn, int depth = 0)
  {
    if (!node || depth > max_world_node_depth)
    {
      return true;
    }

    if (auto *entity = node->entity)
    {
      if (!type.has_value() || entity->type == *type)
      {
        if (!fn(entity))
        {
          return false;
        }
      }
    }

    for (auto child = node->children.begin(); child != node->children.end(); child++)
    {
      if (!for_each_entity(*child, type, fn, depth + 1))
      {
        return false;
      }
    }
    return true;
  }

  template <typename Fn>
  bool for_each_object_entity(Fn &&fn)
  {
    if (!Api::in_game())
    {
      return true;
    }

    return for_each_entity(Api::raw_world_root(), std::nullopt, [&](Entity *entity)
    {
      if (!entity)
      {
        return true;
      }
      if (entity->type == EntityType::object1 || entity->type == EntityType::object2)
      {
        return fn(entity);
      }
      return true;
    });
  }

  template <typename Fn>
  void for_each_player(Fn &&fn)
  {
    if (auto cache = Api::raw_player_update_cache())
    {
      for (auto it = cache->updates.begin(); it != cache->updates.end(); it++)
      {
        if (auto update = *it)
        {
          if (auto player = update->player)
          {
            if (!fn(ApiPlayer(player)))
            {
              return;
            }
          }
        }
      }
    }
  }

  template <typename Fn>
  void for_each_npc(Fn &&fn)
  {
    if (auto cache = Api::raw_npc_update_cache())
    {
      for (uint64_t i = 0; i < cache->size; i++)
      {
        if (auto update = cache->npcs[i])
        {
          if (auto npc = update->npc)
          {
            if (!fn(ApiNpc(npc)))
            {
              return;
            }
          }
        }
      }
    }
  }

  template <typename Fn>
  void for_each_object(Fn &&fn)
  {
    for_each_object_entity([&](Entity *entity)
    {
      return fn(ApiObject(entity));
    });
  }

  template <typename T, typename Each>
  std::optional<T> closest_from(Each &&each, Vec2<uint32_t> from, uint32_t max_dist)
  {
    std::optional<T> best;
    uint32_t best_dist = 0;
    each([&](T item)
    {
      auto dist = Api::chebyshev(from, item.tile_position());
      if (dist > max_dist)
      {
        return;
      }
      if (!best || dist < best_dist)
      {
        best = std::move(item);
        best_dist = dist;
      }
    });
    return best;
  }
} // namespace crs::detail
