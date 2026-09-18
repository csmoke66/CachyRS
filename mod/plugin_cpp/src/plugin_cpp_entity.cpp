#include "plugin_cpp.h"

#include <cctype>
#include <cstring>

namespace crs
{
  namespace
  {
    constexpr size_t max_game_string = 256;

    std::string game_string(const char *s)
    {
      if (!s)
      {
        return {};
      }

      size_t length = 0;
      while (length < max_game_string && s[length] != '\0')
      {
        length++;
      }

      return std::string(s, length);
    }
  } // namespace

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

  uint32_t ApiEntity::distance_to(Vec2<uint32_t> tile) const
  {
    return Api::chebyshev(tile_position(), tile);
  }

  uint32_t ApiEntity::distance_to(const ApiEntity &other) const
  {
    return distance_to(other.tile_position());
  }

  bool ApiEntity::same_plane(const ApiEntity &other) const
  {
    return plane() == other.plane();
  }

  uint32_t ApiEntity::plane() const
  {
    if (!entity)
    {
      return 0;
    }

    return entity->plane;
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
      for (auto it = cache->updates.begin(); it != cache->updates.end(); it++)
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

    return game_string(named->name.c_str());
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

    return !named->animation_queue.empty();
  }

  int32_t ApiNamedEntity::animation_id() const
  {
    if (!named)
    {
      return -1;
    }

    if (named->animation_queue.empty())
    {
      return -1;
    }

    return static_cast<int32_t>(*named->animation_queue.front());
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

    return !queue->points.empty();
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

    for (auto bar = status->bars.begin(); bar != status->bars.end(); bar++)
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

  std::optional<ApiObject> ApiEntity::as_object() const
  {
    if (!entity)
    {
      return std::nullopt;
    }

    if (entity->type != EntityType::object1 && entity->type != EntityType::object2)
    {
      return std::nullopt;
    }

    return ApiObject(entity);
  }

  namespace
  {
    std::string cache_string_text(const CacheString &cs)
    {
      if (cs.pointer_to_string)
      {
        auto text = game_string(cs.pointer_to_string->c_str());
        if (!text.empty())
        {
          return text;
        }
      }
      return game_string(cs.string.c_str());
    }

    bool option_name_equal(std::string_view a, std::string_view b)
    {
      if (a.size() != b.size())
      {
        return false;
      }
      for (size_t i = 0; i < a.size(); i++)
      {
        auto ca = static_cast<unsigned char>(a[i]);
        auto cb = static_cast<unsigned char>(b[i]);
        if (std::tolower(ca) != std::tolower(cb))
        {
          return false;
        }
      }
      return true;
    }

    bool is_blank_or_examine(std::string_view text)
    {
      if (text.empty())
      {
        return true;
      }
      return option_name_equal(text, "Examine") || option_name_equal(text, "Examine...");
    }
  } // namespace

  ApiObject::ApiObject(Entity *entity) : ApiEntity(entity)
  {
  }

  bool ApiObject::is_obj1() const
  {
    return valid() && type() == EntityType::object1;
  }

  bool ApiObject::is_obj2() const
  {
    return valid() && type() == EntityType::object2;
  }

  Obj1 *ApiObject::raw_obj1() const
  {
    return is_obj1() ? static_cast<Obj1 *>(raw()) : nullptr;
  }

  Obj2 *ApiObject::raw_obj2() const
  {
    return is_obj2() ? static_cast<Obj2 *>(raw()) : nullptr;
  }

  ObjectCacheDesc *ApiObject::cache() const
  {
    if (auto *o1 = raw_obj1())
    {
      return o1->cache_buffer.body;
    }
    if (auto *o2 = raw_obj2())
    {
      return o2->cache_buffer.body;
    }
    return nullptr;
  }

  uint32_t ApiObject::id() const
  {
    if (auto *body = cache())
    {
      return body->id;
    }
    if (auto *o1 = raw_obj1())
    {
      return o1->id_1;
    }
    return 0;
  }

  uint32_t ApiObject::interact_id() const
  {
    if (auto *o1 = raw_obj1())
    {
      if (o1->id_2 != static_cast<uint32_t>(-1))
      {
        return o1->id_2;
      }
      return o1->id_1;
    }
    if (auto *body = cache())
    {
      return body->id;
    }
    return 0;
  }

  bool ApiObject::matches_id(uint32_t object_id) const
  {
    return id() == object_id || interact_id() == object_id;
  }

  bool ApiObject::matches_name(std::string_view object_name) const
  {
    if (object_name.empty() || !cache())
    {
      return false;
    }
    return option_name_equal(name(), object_name);
  }

  std::string ApiObject::name() const
  {
    auto *body = cache();
    if (!body || !body->name)
    {
      return {};
    }

    return game_string(body->name->c_str());
  }

  Vec2<uint32_t> ApiObject::tile_position() const
  {
    if (auto *o1 = raw_obj1())
    {
      return o1->tile_position;
    }
    if (auto *o2 = raw_obj2())
    {
      return o2->tile_position;
    }
    return ApiEntity::tile_position();
  }

  std::string ApiObject::option_text(uint32_t index) const
  {
    auto *body = cache();
    if (!body || index >= 6)
    {
      return {};
    }
    return cache_string_text(body->options[index]);
  }

  std::vector<std::pair<uint32_t, std::string>> ApiObject::options() const
  {
    std::vector<std::pair<uint32_t, std::string>> out;
    auto *body = cache();
    if (!body)
    {
      return out;
    }

    for (uint32_t i = 0; i < 6; i++)
    {
      auto text = cache_string_text(body->options[i]);
      if (is_blank_or_examine(text))
      {
        continue;
      }
      out.emplace_back(i, std::move(text));
    }
    return out;
  }

  std::optional<uint32_t> ApiObject::option_index(std::string_view option_name) const
  {
    auto *body = cache();
    if (!body || option_name.empty())
    {
      return std::nullopt;
    }

    for (uint32_t i = 0; i < 6; i++)
    {
      auto text = cache_string_text(body->options[i]);
      if (!text.empty() && option_name_equal(text, option_name))
      {
        return i;
      }
    }
    return std::nullopt;
  }

  void ApiObject::interact(uint32_t option) const
  {
    if (!valid())
    {
      return;
    }

    auto tile = tile_position();
    Api::interact_object(interact_id(), tile.x, tile.y, option);
  }

  bool ApiObject::interact(std::string_view option_name) const
  {
    if (!valid() || !cache())
    {
      return false;
    }
    auto idx = option_index(option_name);
    if (!idx)
    {
      return false;
    }
    interact(*idx);
    return true;
  }
} // namespace crs
