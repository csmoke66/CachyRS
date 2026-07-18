#include "game_dom.h"
#include "cachy.h"
#include "not_cachy.h"

namespace crs
{

    PlayerDomNode::PlayerDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : ValueDomNode<Entity *>(tree, id, type)
    {
    }

    PlayersDomNode::PlayersDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : GameContainerNode<PlayerDomNode>(tree, id, type)
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
            iterate_players_update([this](const NamedEntity *entity)
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

                        auto vt_address_node = std::make_unique<PointerDomValue>("VT address", *((void**)entity));
                        {
                            vt_address_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(vt_address_node));
                        }
                        
                        new_dom_node->values.push_back(std::make_unique<StringDomValue>("name", entity->name.c_str()));
                        
                        auto x_node = std::make_unique<FloatDomValue>("x", entity->position.x);
                        {
                            x_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(x_node));
                        }

                        auto y_node = std::make_unique<FloatDomValue>("y", entity->position.y);
                        {
                            y_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(y_node));
                        }

                        auto z_node = std::make_unique<FloatDomValue>("z", entity->position.z);
                        {
                            z_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(z_node));
                        }

                        for (auto i = entity->animation_queue.begin; i != entity->animation_queue.end; i++)
                        {
                            auto name = std::format("Animation Id #{}", *(i));
                            
                            auto animation_id_node = std::make_unique<FloatDomValue>(name, entity->position.z);
                            animation_id_node->mark_hidden();

                            new_dom_node->values.push_back(std::move(animation_id_node));
                        }

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

}