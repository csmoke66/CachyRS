#include "cachy.h"
#include "game_dom.h"
#include "not_cachy.h"

namespace crs
{
  StatDomNode::StatDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : DomNode(tree, id, type)
  {
  }

  StatsDomNode::StatsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<StatDomNode>(tree, id, type)
  {
  }

  void StatsDomNode::update()
  {
    iterate_typed_children([this](StatDomNode *node)
    {
      node->seen = false;
      return false;
    });

    auto ws = NRS.variable_cache();
    if (auto lpv = ws->local_player_variables)
    {
      for (auto i = 0; i < lpv->stat_count; i++)
      {
        auto stat = lpv->stats[i];

        auto id = std::format("stat_{}", i);
        auto child = find_typed_child(id);
        if (!child)
        {
          auto new_dom_node = std::make_shared<StatDomNode>(tree, id, "stat");
          new_dom_node->add_value(std::make_unique<UInt32DomValue>("id", i));

          auto current_level_node = std::make_unique<UInt32DomValue>("current level", stat.current_level);
          current_level_node->mark_hidden();
          new_dom_node->add_value(std::move(current_level_node));

          auto max_level_node = std::make_unique<UInt32DomValue>("max level", stat.max_level);
          max_level_node->mark_hidden();
          new_dom_node->add_value(std::move(max_level_node));

          auto experience_node = std::make_unique<UInt32DomValue>("experience", stat.experience);
          experience_node->mark_hidden();
          new_dom_node->add_value(std::move(experience_node));

          new_dom_node->parent = shared_from_this();
          children[id] = new_dom_node;
        }
        else
        {
          auto current_level_node = child->find_value<UInt32DomValue>("current level");
          auto max_level_node = child->find_value<UInt32DomValue>("max level");
          auto experience_node = child->find_value<UInt32DomValue>("experience");

          if (current_level_node->val != stat.current_level)
          {
            current_level_node->val = stat.current_level;
            current_level_node->mark_dirty();

            child->mark_dirty();
          }

          if (max_level_node->val != stat.max_level)
          {
            max_level_node->val = stat.max_level;
            max_level_node->mark_dirty();

            child->mark_dirty();
          }

          if (experience_node->val != stat.experience)
          {
            experience_node->val = stat.experience;
            experience_node->mark_dirty();

            child->mark_dirty();
          }
          child->seen = true;
        }
      }
    }

    iterate_typed_children([this](StatDomNode *node)
    {
      auto remove = !node->seen;
      return remove;
    });
  }
} // namespace crs