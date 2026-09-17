#include "plugin_cpp.h"

namespace crs
{
  ApiEntity::ApiEntity(Entity *entity) : entity(entity)
  {
  }

  bool ApiEntity::valid() const
  {
    return entity != nullptr;
  }

  ApiEntity::operator bool() const
  {
    return valid();
  }

  Entity *ApiEntity::raw() const
  {
    return entity;
  }

  EntityType ApiEntity::type() const
  {
    if (!entity)
    {
      return EntityType::object1;
    }

    return entity->type;
  }

  WorldNode *ApiEntity::node() const
  {
    if (!entity)
    {
      return nullptr;
    }

    return entity->parent;
  }

  Vec3<float> ApiEntity::scene_position() const
  {
    if (auto n = node())
    {
      return n->pos_avg;
    }

    return { 0.f, 0.f, 0.f };
  }

  Vec2<uint32_t> ApiEntity::tile_position() const
  {
    auto scene_pos = scene_position();
    return Vec2<uint32_t>(static_cast<uint32_t>(scene_pos.x / 512.f), static_cast<uint32_t>(scene_pos.z / 512.f));
  }

  bool ApiEntity::rendered() const
  {
    if (auto n = node())
    {
      return (n->flags & WorldNodeFlag::has_entity) == WorldNodeFlag::has_entity;
    }

    return false;
  }

  void ApiEntity::set_rendered(bool visible)
  {
    if (auto n = node())
    {
      if (visible)
      {
        n->flags |= WorldNodeFlag::has_entity;
      }
      else
      {
        n->flags &= ~WorldNodeFlag::has_entity;
      }
    }
  }

  std::optional<ApiPlayer> ApiEntity::as_player() const
  {
    if (!entity || entity->type != EntityType::player)
    {
      return std::nullopt;
    }

    if (auto cache = Api::raw_player_update_cache())
    {
      for (auto it = cache->updates.begin; it != cache->updates.end; it++)
      {
        if (auto update = *it)
        {
          if (update->player == entity)
          {
            return ApiPlayer(update->player);
          }
        }
      }
    }

    return std::nullopt;
  }

  std::optional<ApiNpc> ApiEntity::as_npc() const
  {
    if (!entity || entity->type != EntityType::npc)
    {
      return std::nullopt;
    }

    if (auto cache = Api::raw_npc_update_cache())
    {
      for (uint64_t i = 0; i < cache->size; i++)
      {
        if (auto update = cache->npcs[i])
        {
          if (update->npc == entity)
          {
            return ApiNpc(update->npc);
          }
        }
      }
    }

    return std::nullopt;
  }

  ApiNamedEntity::ApiNamedEntity(NamedEntity *named) : ApiEntity(named),
                                                       named(named)
  {
  }

  NamedEntity *ApiNamedEntity::raw_named() const
  {
    return named;
  }

  int32_t ApiNamedEntity::server_index() const
  {
    if (!named)
    {
      return -1;
    }

    return named->server_index;
  }

  std::string ApiNamedEntity::name() const
  {
    if (!named)
    {
      return "INVALID";
    }

    return named->name.str();
  }

  Vec3<float> ApiNamedEntity::scene_position() const
  {
    if (!named)
    {
      return ApiEntity::scene_position();
    }

    return named->position;
  }

  bool ApiNamedEntity::animation_playing() const
  {
    if (!named)
    {
      return false;
    }

    return named->animation_queue.begin != named->animation_queue.end;
  }

  int32_t ApiNamedEntity::animation_id() const
  {
    if (!named)
    {
      return -1;
    }

    if (named->animation_queue.begin == named->animation_queue.end)
    {
      return -1;
    }

    return static_cast<int32_t>(*named->animation_queue.begin);
  }

  bool ApiNamedEntity::moving() const
  {
    if (!named)
    {
      return false;
    }

    auto queue = named->movement_queue;
    if (!queue)
    {
      return false;
    }

    return queue->points.size != 0;
  }

  std::vector<StatusBarView> ApiNamedEntity::status_bars() const
  {
    std::vector<StatusBarView> bars;
    if (!named)
    {
      return bars;
    }

    auto status = named->status;
    if (!status)
    {
      return bars;
    }

    for (auto bar = status->bars.begin; bar != status->bars.end; bar++)
    {
      auto data = bar->data;
      if (!data)
      {
        continue;
      }

      StatusBarView view;
      view.value = data->value;
      view.display_time = data->display_time;
      if (auto config = data->config)
      {
        view.id = config->id;
      }

      bars.push_back(view);
    }

    return bars;
  }

  std::optional<uint8_t> ApiNamedEntity::status(uint32_t bar_id) const
  {
    for (auto &bar : status_bars())
    {
      if (bar.id == bar_id)
      {
        return bar.value;
      }
    }

    return std::nullopt;
  }

  ApiPlayer::ApiPlayer(Player *player) : ApiNamedEntity(player),
                                         player(player)
  {
  }

  ApiPlayer ApiPlayer::invalid()
  {
    return ApiPlayer(nullptr);
  }

  Player *ApiPlayer::raw_player() const
  {
    return player;
  }

  int32_t ApiPlayer::combat_level() const
  {
    if (!player)
    {
      return -1;
    }

    return player->combat_level;
  }

  int32_t ApiPlayer::skill_level() const
  {
    if (!player)
    {
      return -1;
    }

    return player->skill_level;
  }

  bool ApiPlayer::is_self() const
  {
    return player && player == Api::raw_self();
  }

  bool ApiPlayer::is_friend() const
  {
    return player && Api::raw_is_friend(player);
  }

  ApiNpc::ApiNpc(Npc *npc) : ApiNamedEntity(npc),
                             npc(npc)
  {
  }

  Npc *ApiNpc::raw_npc() const
  {
    return npc;
  }

  int32_t ApiNpc::cache_id() const
  {
    if (!npc)
    {
      return -1;
    }

    return npc->cache_id;
  }

  uint32_t ApiNpc::visible_level() const
  {
    if (!npc)
    {
      return 0;
    }

    return npc->visible_level;
  }

  void ApiNpc::interact(uint32_t option) const
  {
    if (!valid())
    {
      return;
    }

    Api::interact_npc(static_cast<uint32_t>(server_index()), option);
  }
} // namespace crs
