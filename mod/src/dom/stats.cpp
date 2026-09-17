#include "cachy.h"
#include "dom_sync.h"
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
    auto cache = NRS.variable_cache();
    if (!cache)
    {
      return;
    }

    begin_sync();

    if (auto lpv = cache->local_player_variables)
    {
      if (lpv->stats)
      {
        for (auto i = 0u; i < lpv->stat_count; i++)
        {
          auto stat = lpv->stats[i];
          auto key = static_cast<uintptr_t>(i);
          if (auto child = touch_typed_key(key))
          {
            child->set_value<UInt32DomValue>("current level", stat.current_level);
            child->set_value<UInt32DomValue>("max level", stat.max_level);
            child->set_value<UInt32DomValue>("experience", stat.experience);
          }
          else
          {
            auto node = std::make_shared<StatDomNode>(tree, make_numeric_id("stat_", i), "stat");
            node->add_value(std::make_unique<UInt32DomValue>("id", i));
            add_hidden_uint32(*node, "current level", stat.current_level);
            add_hidden_uint32(*node, "max level", stat.max_level);
            add_hidden_uint32(*node, "experience", stat.experience);
            add_keyed_child(key, node);
          }
        }
      }
    }

    end_sync();
  }
} // namespace crs
