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
                child->seen = true;
            }
            else
            {
                auto new_dom_node = std::make_shared<ItemDomNode>(tree, id, "item");
                new_dom_node->values.push_back(std::make_unique<Int32DomValue>("id", i->id));
                new_dom_node->values.push_back(std::make_unique<Int32DomValue>("amount", i->amount));
                new_dom_node->parent = shared_from_this();

                children[id] = new_dom_node;
            }

            slot += 1;
        }

        // clang-format off
        iterate_typed_children([this](ItemDomNode *node)
        { 
            return !node->seen;
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
                    new_dom_node->values.push_back(std::move(address_node));
                }

                new_dom_node->values.push_back(std::make_unique<UInt32DomValue>("id", i->id));
                new_dom_node->parent = shared_from_this();

                children[id] = new_dom_node;
            }
        }

        // clang-format off
        iterate_typed_children([this](ItemContainerDomNode *node)
        { 
            return !node->seen;
        });
        // clang-format on
    }
}