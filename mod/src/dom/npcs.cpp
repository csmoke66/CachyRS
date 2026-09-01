#include "game_dom.h"
#include "cachy.h"
#include "not_cachy.h"

namespace crs
{

    NpcDomNode::NpcDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : ValueDomNode<const Entity *>(tree, id, type)
    {
    }

    NpcsDomNode::NpcsDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : GameContainerNode<NpcDomNode>(tree, id, type)
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
            iterate_npcs_update([this](const NamedEntity *entity)
            {
                if (entity->type == EntityType::npc)
                {
                    auto id = std::format("npc_{}", (void*)entity);
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

                        auto x_value = child->find_value<FloatDomValue>("x");
                        auto y_value = child->find_value<FloatDomValue>("y");
                        auto z_value = child->find_value<FloatDomValue>("z");

                        if (x_value->val != entity->position.x ||
                            y_value->val != entity->position.y ||
                            z_value->val != entity->position.z)
                        {
                            x_value->val = entity->position.x;
                            x_value->mark_dirty();

                            y_value->val = entity->position.y;
                            y_value->mark_dirty();

                            z_value->val = entity->position.z;
                            z_value->mark_dirty();

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

                        auto vt_address_node = std::make_unique<PointerDomValue>("VT address", *((void**)entity));
                        {
                            vt_address_node->mark_hidden();
                            new_dom_node->add_value(std::move(vt_address_node));
                        }

                        new_dom_node->add_value(std::make_unique<StringDomValue>("name", entity->name.c_str()));
                        
                        auto x_node = std::make_unique<FloatDomValue>("x", entity->position.x);
                        {
                            x_node->mark_hidden();
                            new_dom_node->add_value(std::move(x_node));
                        }

                        auto y_node = std::make_unique<FloatDomValue>("y", entity->position.y);
                        {
                            y_node->mark_hidden();
                            new_dom_node->add_value(std::move(y_node));
                        }

                        auto z_node = std::make_unique<FloatDomValue>("z", entity->position.z);
                        {
                            z_node->mark_hidden();
                            new_dom_node->add_value(std::move(z_node));
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
            // clang-format on
        }

        // clang-format off
        iterate_typed_children([this](NpcDomNode *node)
        { 
            auto remove = !node->seen;
            if (remove)
            {
                RS.stats.npc_dom_nodes_removed_recent += 1;
            }
            return remove;
        });
        // clang-format on
    }

}