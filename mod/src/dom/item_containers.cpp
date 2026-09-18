#include "cachy.h"
#include "dom_sync.h"
#include "game_dom.h"
#include "not_cachy.h"

namespace crs
{
  ItemDomNode::ItemDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : ValueDomNode<Item>(tree, id, type)
  {
  }

  ItemContainerDomNode::ItemContainerDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : TypedChildrenDomNode<ItemDomNode>(tree, id, type)
  {
  }

  ItemContainersDomNode::ItemContainersDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<ItemContainerDomNode>(tree, id, type)
  {
  }

  void ItemContainerDomNode::update(const std::string &parent_id, const ItemContainer &container)
  {
    begin_sync();

    auto slot = 0u;
    for (auto item = container.items.begin(); item != container.items.end(); item++)
    {
      auto key = static_cast<uintptr_t>(slot);
      if (auto child = touch_typed_key(key))
      {
        child->set_value<Int32DomValue>("id", item->id);
        child->set_value<Int32DomValue>("amount", item->amount);
      }
      else
      {
        auto node = std::make_shared<ItemDomNode>(tree, make_numeric_id(parent_id + "_", slot), "item");
        node->add_value(std::make_unique<Int32DomValue>("id", item->id));
        node->add_value(std::make_unique<Int32DomValue>("amount", item->amount));
        if (item->id != -1)
        {
          add_hidden_pointer(*node, "desc", NRS.get_cache_data<void>(CacheIndexOrdinal::items, item->id));
        }

        add_keyed_child(key, node);
        RS.stats.item_dom_nodes_created += 1;
        RS.stats.item_dom_nodes_created_recent += 1;
      }

      slot += 1;
    }

    end_sync();
    if (needs_prune)
    {
      RS.stats.item_dom_nodes_removed_recent += count_deleted_children();
    }
  }

  void ItemContainersDomNode::update()
  {
    auto item_cache = NRS.item_cache();
    if (!item_cache)
    {
      return;
    }

    begin_sync();

    for (auto container = item_cache->containers.begin(); container != item_cache->containers.end(); container++)
    {
      auto key = static_cast<uintptr_t>(container->id);
      if (auto child = touch_typed_key(key))
      {
        child->update(child->id, *container);
      }
      else
      {
        auto id = make_numeric_id("item_container_", container->id);
        auto node = std::make_shared<ItemContainerDomNode>(tree, id, "item_container");
        add_hidden_pointer(*node, "address", container);
        node->add_value(std::make_unique<UInt32DomValue>("id", container->id));
        add_keyed_child(key, node);
        node->update(id, *container);

        RS.stats.item_container_dom_nodes_created += 1;
        RS.stats.item_container_dom_nodes_created_recent += 1;
      }
    }

    end_sync();
    if (needs_prune)
    {
      RS.stats.item_container_dom_nodes_removed_recent += count_deleted_children();
    }
  }
} // namespace crs
