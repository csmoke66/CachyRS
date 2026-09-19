#include "cachy.h"
#include "dom_sync.h"
#include "game_dom.h"
#include "not_cachy.h"

namespace crs
{
  WorldSettingDomNode::WorldSettingDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : DomNode(tree, id, type)
  {
  }

  WorldSettingsDomNode::WorldSettingsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<WorldSettingDomNode>(tree, id, type)
  {
  }

  void WorldSettingsDomNode::update()
  {
    auto ws = NRS.world_setting_cache();
    if (!ws || !ws->vars)
    {
      return;
    }

    if (children_by_key.empty())
    {
      children_by_key.reserve(16384);
      child_order.reserve(16384);
      children.reserve(16384);
    }

    begin_sync();

    for (auto i = 0u; i < ws->count; i++)
    {
      auto setting = ws->vars[i];
      while (setting)
      {
        if (!setting->body.initialized)
        {
          auto key = static_cast<uintptr_t>(setting->id);
          if (auto child = touch_typed_key(key))
          {
            if (child->value)
            {
              child->value->set(setting->body.value);
            }
          }
          else
          {
            auto node = std::make_shared<WorldSettingDomNode>(tree, make_numeric_id("world_setting_", setting->id), "world_setting");
            add_inlined_uint32(*node, "id", setting->id);
            add_uint32(*node, "value", setting->body.value);
            node->value = node->find_value<UInt32DomValue>("value");
            add_keyed_child(key, node);
          }
        }

        setting = setting->body.next;
      }
    }

    end_sync();
  }
} // namespace crs
