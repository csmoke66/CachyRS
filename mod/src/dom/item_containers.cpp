#include "game_dom.h"
#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    ItemDomNode::ItemDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : ValueDomNode<Item>(tree, id, type)
    {
    }

    ItemContainerDomNode::ItemContainerDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<ItemDomNode>(tree, id, type)
    {
    }

    ItemContainersDomNode::ItemContainersDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : GameContainerNode<ItemContainerDomNode>(tree, id, type)
    {
    }

    void ItemContainerDomNode::update(const std::string &parent_id, const ItemContainer &container)
    {
        // clang-format off
        iterate_typed_children([this](ItemDomNode* node)
        {
            node->seen = false;
            return false;
        });
        // clang-format on

        auto slot = 0;
        for (auto i = container.items.begin; i != container.items.end; i++)
        {
            auto id = parent_id + std::string("_") + std::format("{}", slot);
            auto child = find_typed_child(id);
            if (child)
            {
                auto id_value = child->find_value<Int32DomValue>("id");
                auto amount_value = child->find_value<Int32DomValue>("amount");

                if (id_value->val != i->id || amount_value->val != i->amount)
                {
                    id_value->val = i->id;
                    id_value->mark_dirty();

                    amount_value->val = i->amount;
                    amount_value->mark_dirty();
                    
                    child->mark_dirty();
                }

                child->seen = true;
            }
            else
            {
                auto new_dom_node = std::make_shared<ItemDomNode>(tree, id, "item");
                new_dom_node->add_value(std::make_unique<Int32DomValue>("id", i->id));
                new_dom_node->add_value(std::make_unique<Int32DomValue>("amount", i->amount));
                new_dom_node->parent = shared_from_this();

                children[id] = new_dom_node;
                RS.stats.item_dom_nodes_created += 1;
                RS.stats.item_dom_nodes_created_recent += 1;
            }

            slot += 1;
        }

        // clang-format off
        iterate_typed_children([this](ItemDomNode *node)
        { 
            auto remove = !node->seen;
            if (remove)
            {
                RS.stats.item_dom_nodes_removed_recent += 1;
            }
            return remove;
        });
        // clang-format on
    }

    void ItemContainersDomNode::update()
    {
        auto item_cache = NRS.item_cache();
        if (!item_cache)
        {
            return;
        }

        // clang-format off
        iterate_typed_children([this](ItemContainerDomNode* node)
        {
            node->seen = false;
            return false;
        });
        // clang-format on

        for (auto i = item_cache->containers.begin; i != item_cache->containers.end; i++)
        {
            auto id = std::string("item_container_") + std::format("{}", i->id);
            auto child = find_typed_child(id);
            if (child)
            {
                child->update(id, *i);
                child->seen = true;
            }
            else
            {
                auto new_dom_node = std::make_shared<ItemContainerDomNode>(tree, id, "item_container");

                auto address_node = std::make_unique<PointerDomValue>("address", i);
                {
                    address_node->mark_hidden();
                    new_dom_node->add_value(std::move(address_node));
                }

                new_dom_node->add_value(std::make_unique<UInt32DomValue>("id", i->id));
                new_dom_node->parent = shared_from_this();

                children[id] = new_dom_node;

                RS.stats.item_container_dom_nodes_created += 1;
                RS.stats.item_container_dom_nodes_created_recent += 1;
            }
        }

        // clang-format off
        iterate_typed_children([this](ItemContainerDomNode *node)
        { 
            auto remove = !node->seen;
            if (remove)
            {
                RS.stats.item_container_dom_nodes_removed_recent += 1;
            }
            return remove;
        });
        // clang-format on
    }
}