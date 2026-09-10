#include "cachy.h"
#include "game_dom.h"
#include "not_cachy.h"

namespace crs
{

  NpcDomNode::NpcDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : ValueDomNode<const Entity *>(tree, id, type)
  {
  }

  NpcsDomNode::NpcsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<NpcDomNode>(tree, id, type)
  {
  }

  void NpcsDomNode::update()
  {
    auto engine = RS.get_globals()->engine;

    iterate_typed_children([this](NpcDomNode *node)
    {
      node->seen = false;
      return false;
    });

    if (engine->state == GameState::in_game)
    {
      iterate_npcs_update([this](const NamedEntity *entity)
      {
        if (entity->type == EntityType::npc)
        {
          auto id = std::format("npc_{}", (void *)entity);
          auto child = find_typed_child(id);
          if (child)
          {
            auto name_value = child->find_value<StringDomValue>("name");
            if (entity->name.str() != name_value->val)
            {
              name_value->val = entity->name.str();
              name_value->mark_dirty();

              child->mark_dirty();
            }

            auto sx_value = child->find_value<FloatDomValue>("scene x");
            auto sy_value = child->find_value<FloatDomValue>("scene y");
            auto sz_value = child->find_value<FloatDomValue>("scene z");

            auto tx_value = child->find_value<Int32DomValue>("tile x");
            auto ty_value = child->find_value<Int32DomValue>("tile y");

            if (sx_value->val != entity->position.x ||
                sy_value->val != entity->position.y ||
                sz_value->val != entity->position.z)
            {
              sx_value->val = entity->position.x;
              sx_value->mark_dirty();

              sy_value->val = entity->position.y;
              sy_value->mark_dirty();

              sz_value->val = entity->position.z;
              sz_value->mark_dirty();

              tx_value->val = static_cast<int32_t>(entity->position.x / 512.f);
              tx_value->mark_dirty();

              ty_value->val = static_cast<int32_t>(entity->position.z / 512.f);
              ty_value->mark_dirty();

              child->mark_dirty();
            }

            child->seen = true;
          }
          else
          {
            auto new_dom_node = std::make_shared<NpcDomNode>(tree, id, "npc");
            new_dom_node->value = entity;

            auto address_node = std::make_unique<PointerDomValue>("address", entity);
            {
              address_node->mark_hidden();
              new_dom_node->add_value(std::move(address_node));
            }

            auto vt_address_node = std::make_unique<PointerDomValue>("VT address", *((void **)entity));
            {
              vt_address_node->mark_hidden();
              new_dom_node->add_value(std::move(vt_address_node));
            }

            new_dom_node->add_value(std::make_unique<StringDomValue>("name", entity->name.c_str()));

            auto sx_node = std::make_unique<FloatDomValue>("scene x", entity->position.x);
            {
              sx_node->mark_hidden();
              new_dom_node->add_value(std::move(sx_node));
            }

            auto sy_node = std::make_unique<FloatDomValue>("scene y", entity->position.y);
            {
              sy_node->mark_hidden();
              new_dom_node->add_value(std::move(sy_node));
            }

            auto sz_node = std::make_unique<FloatDomValue>("scene z", entity->position.z);
            {
              sz_node->mark_hidden();
              new_dom_node->add_value(std::move(sz_node));
            }

            auto tx_node = std::make_unique<Int32DomValue>("tile x", static_cast<int32_t>(entity->position.x / 512.f));
            {
              tx_node->mark_hidden();
              new_dom_node->add_value(std::move(tx_node));
            }

            auto ty_node = std::make_unique<Int32DomValue>("tile y", static_cast<int32_t>(entity->position.z / 512.f));
            {
              ty_node->mark_hidden();
              new_dom_node->add_value(std::move(ty_node));
            }

            for (auto i = entity->animation_queue.begin; i != entity->animation_queue.end; i++)
            {
              auto name = std::format("Animation Id #{}", *(i));

              auto animation_id_node = std::make_unique<FloatDomValue>(name, entity->position.z);
              animation_id_node->mark_hidden();

              new_dom_node->add_value(std::move(animation_id_node));
            }

            new_dom_node->parent = shared_from_this();

            children[id] = new_dom_node;
            RS.stats.npc_dom_nodes_created += 1;
            RS.stats.npc_dom_nodes_created_recent += 1;
          }
        }
      });
    }

    iterate_typed_children([this](NpcDomNode *node)
    {
      auto remove = !node->seen;
      if (remove)
      {
        RS.stats.npc_dom_nodes_removed_recent += 1;
      }
      return remove;
    });
  }

} // namespace crs