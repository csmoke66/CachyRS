#include "game_dom.h"
#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    PlayerDomNode::PlayerDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : ValueDomNode<Entity *>(tree, id, type)
    {
    }

    PlayersDomNode::PlayersDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<PlayerDomNode>(tree, id, type)
    {
    }

    void PlayersDomNode::update()
    {
        auto engine = RS.get_globals()->engine;

        // clang-format off
        iterate_typed_children([this](PlayerDomNode* node)
        {
            node->seen = false;
            return false;
        });
        // clang-format on

        if (engine->state == GameState::in_game)
        {
            // clang-format off
            iterate_players_update([this](const Entity *entity)
            {
                if (entity->type == EntityType::player)
                {
                    auto id = std::format("player_{}", (void*)entity);
                    auto child = find_typed_child(id);
                    if (!child)
                    {
                        auto new_dom_node = std::make_shared<PlayerDomNode>(tree, id, "player");
                        new_dom_node->player = entity;

                        auto address_node = std::make_unique<PointerDomValue>("address", entity);
                        {
                            address_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(address_node));
                        }
                        
                        new_dom_node->values.push_back(std::make_unique<StringDomValue>("name", entity->name.c_str()));
                        
                        auto x_node = std::make_unique<FloatDomValue>("x", entity->position.x);
                        x_node->mark_hidden();
                        new_dom_node->values.push_back(std::move(x_node));

                        auto y_node = std::make_unique<FloatDomValue>("y", entity->position.y);
                        y_node->mark_hidden();
                        new_dom_node->values.push_back(std::move(y_node));

                        auto z_node = std::make_unique<FloatDomValue>("z", entity->position.z);
                        z_node->mark_hidden();
                        new_dom_node->values.push_back(std::move(z_node));

                        new_dom_node->parent = shared_from_this();
                        
                        children[id] = new_dom_node;
                    }
                    else
                    {
                        child->seen = true;
                    }
                } 
            });
            // clang-format on
        }

        // clang-format off
        iterate_typed_children([this](PlayerDomNode *node)
        { 
            return !node->seen;
        });
        // clang-format on
    }

    void PlayersDomNode::prune()
    {
        for (auto it = children.begin(); it != children.end();)
        {
            if (it->second->deleted)
            {
                tree->remove_dom_node(it->second);
                it = children.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    NpcDomNode::NpcDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : ValueDomNode<Entity *>(tree, id, type)
    {
    }

    NpcsDomNode::NpcsDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<NpcDomNode>(tree, id, type)
    {
    }

    void NpcsDomNode::update()
    {
        auto engine = RS.get_globals()->engine;

        // clang-format off
        iterate_typed_children([this](NpcDomNode* node)
        {
            node->seen = false;
            return false;
        });
        // clang-format on

        if (engine->state == GameState::in_game)
        {
            // clang-format off
            iterate_npcs_update([this](const Entity *entity)
            {
                if (entity->type == EntityType::npc)
                {
                    auto id = std::format("npc_{}", (void*)entity);
                    auto child = find_typed_child(id);
                    if (!child)
                    {
                        auto new_dom_node = std::make_shared<NpcDomNode>(tree, id, "npc");
                        new_dom_node->npc = entity;

                        auto address_node = std::make_unique<PointerDomValue>("address", entity);
                        {
                            address_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(address_node));
                        }

                        new_dom_node->values.push_back(std::make_unique<StringDomValue>("name", entity->name.c_str()));
                        
                        auto x_node = std::make_unique<FloatDomValue>("x", entity->position.x);
                        x_node->mark_hidden();
                        new_dom_node->values.push_back(std::move(x_node));

                        auto y_node = std::make_unique<FloatDomValue>("y", entity->position.y);
                        y_node->mark_hidden();
                        new_dom_node->values.push_back(std::move(y_node));

                        auto z_node = std::make_unique<FloatDomValue>("z", entity->position.z);
                        z_node->mark_hidden();
                        new_dom_node->values.push_back(std::move(z_node));

                        new_dom_node->parent = shared_from_this();
                        
                        children[id] = new_dom_node;
                    }
                    else
                    {
                        child->seen = true;
                    }
                } 
            });
            // clang-format on
        }

        // clang-format off
        iterate_typed_children([this](NpcDomNode *node)
        { 
            return !node->seen;
        });
        // clang-format on
    }

    void NpcsDomNode::prune()
    {
        for (auto it = children.begin(); it != children.end();)
        {
            if (it->second->deleted)
            {
                tree->remove_dom_node(it->second);
                it = children.erase(it);
            }
            else
            {
                it++;
            }
        }
    }

    ItemDomNode::ItemDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : ValueDomNode<Item>(tree, id, type)
    {
    }

    ItemContainerDomNode::ItemContainerDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<ItemDomNode>(tree, id, type)
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

    ItemContainersDomNode::ItemContainersDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<ItemContainerDomNode>(tree, id, type)
    {
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

    void ItemContainersDomNode::prune()
    {
        for (auto it = children.begin(); it != children.end();)
        {
            if (it->second->deleted)
            {
                tree->remove_dom_node(it->second);
                it = children.erase(it);
            }
            else
            {
                it++;
            }
        }
    }
}