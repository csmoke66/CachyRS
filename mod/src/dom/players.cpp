#include "cachy.h"
#include "dom_sync.h"
#include "game_dom.h"
#include "not_cachy.h"

namespace crs
{
  PlayerDomNode::PlayerDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : ValueDomNode<const Entity *>(tree, id, type)
  {
  }

  PlayersDomNode::PlayersDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<PlayerDomNode>(tree, id, type)
  {
  }

  void PlayersDomNode::update()
  {
    auto engine = RS.get_globals()->engine;
    if (!engine)
    {
      return;
    }

    begin_sync();

    if (engine->state == GameState::in_game)
    {
      iterate_players_update([this](const NamedEntity *entity)
      {
        if (entity->type != EntityType::player)
        {
          return;
        }

        auto key = reinterpret_cast<uintptr_t>(entity);
        if (auto child = touch_typed_key(key))
        {
          sync_named_entity_values(*child, entity);
        }
        else
        {
          auto node = std::make_shared<PlayerDomNode>(tree, make_pointer_id("player_", entity), "player");
          node->value = entity;
          add_named_entity_values(*node, entity);
          add_keyed_child(key, node);
          RS.stats.player_dom_nodes_created += 1;
          RS.stats.player_dom_nodes_created_recent += 1;
        }
      });
    }

    end_sync();
    if (needs_prune)
    {
      RS.stats.player_dom_nodes_removed_recent += count_deleted_children();
    }
  }
} // namespace crs
