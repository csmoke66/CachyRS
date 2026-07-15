#include "dom.h"
#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    DomNode::DomNode(::std::string id, ::std::string type)
    {
        this->id = id;
        this->type = type;
    }

    DomNode *DomNode::find_dom_node(DomNode *current, const std::string &id)
    {
        if (current->id == id)
        {
            return current;
        }

        for (auto child : current->children)
        {
            if (auto node = find_dom_node(child.second.get(), id))
            {
                return node;
            }
        }
        return nullptr;
    }

    DomNode *DomNode::find_dom_node(const std::string &id)
    {
        return find_dom_node(this, id);
    }

    void DomNode::destroy_dom_node(DomNode *child)
    {
        if (child->wrapper_element)
        {
            auto parent = child->wrapper_element->GetParentNode();
            parent->RemoveChild(child->wrapper_element);
        }
    }

    void DomNode::remove_dom_node(const std::string &id)
    {
        auto child = children.find(id);
        if (child != children.end())
        {
            destroy_dom_node(child->second.get());
            children.erase(child);
        }
    }

    void DomNode::reset()
    {
        while (element && element->HasChildNodes())
        {
            element->RemoveChild(element->GetLastChild());
        }
        children.clear();
    }

    NpcDomNode::NpcDomNode(const ::std::string &id, const ::std::string &type) : ValueDomNode<Entity *>(id, type)
    {
    }

    NpcsDomNode::NpcsDomNode(const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<NpcDomNode>(id, type)
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
            if (auto scene = NRS.scene_003())
            {
                if (auto node = scene->world_root)
                {
                    // clang-format off
                    iterate_entities(node, [this](const Entity *entity)
                    {
                        if (entity->type == EntityType::npc)
                        {
                            auto id = std::format("npc_{}", (void*)entity);
                            auto child = find_typed_child(id);
                            if (!child)
                            {
                                LOG(INFO, "Building new dom node");
                                
                                auto new_dom_node = std::make_shared<NpcDomNode>(id, "npc");
                                new_dom_node->values.push_back({"address", std::format("{}", (void*)entity)}); 
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
            }
        }

        // clang-format off
        iterate_typed_children([this](NpcDomNode *node)
        { 
            return !node->seen;
        });
        // clang-format on
    }

    ItemDomNode::ItemDomNode(const ::std::string &id, const ::std::string &type) : ValueDomNode<Item>(id, type)
    {
    }

    ItemContainerDomNode::ItemContainerDomNode(const ::std::string &id, const ::std::string &type) : TypedChildrenDomNode<ItemDomNode>(id, type)
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
                LOG(INFO, "Building new dom node");

                auto new_dom_node = std::make_shared<ItemDomNode>(id, "item");
                new_dom_node->values.push_back({"id", std::format("{}", i->id)});
                new_dom_node->values.push_back({"amount", std::format("{}", i->amount)});
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

    ItemContainersDomNode::ItemContainersDomNode(const ::std::string& id, const ::std::string& type) : TypedChildrenDomNode<ItemContainerDomNode>(id, type)
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
                auto new_dom_node = std::make_shared<ItemContainerDomNode>(id, "item_container");
                new_dom_node->values.push_back({"address", std::format("{}", (void *)i)});
                new_dom_node->values.push_back({"id", std::format("{}", i->id)});
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