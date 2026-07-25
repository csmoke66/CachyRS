#include "game_dom.h"
#include "cachy.h"
#include "not_cachy.h"

namespace crs
{
    WorldSettingDomNode::WorldSettingDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : DomNode(tree, id, type)
    {
    }

    WorldSettingsDomNode::WorldSettingsDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : GameContainerNode<WorldSettingDomNode>(tree, id, type)
    {
    }

    void WorldSettingsDomNode::update()
    {
        // clang-format off
        iterate_typed_children([this](WorldSettingDomNode* node)
        {
            node->seen = false;
            return false;
        });
        // clang-format on

        auto ws = NRS.world_setting_cache();
        for (auto i = 0; i < ws->count; i++)
        {
            auto c = ws->vars[i];
            while (c)
            {
                if (!c->body.uninitialized)
                {
                    auto id = std::format("world_setting_{}", c->id);
                    auto child = find_typed_child(id);
                    if (!child)
                    {
                        auto new_dom_node = std::make_shared<WorldSettingDomNode>(tree, id, "world_setting");
                        new_dom_node->values.push_back(std::make_unique<UInt32DomValue>("id", i));

                        auto value_node = std::make_unique<UInt32DomValue>("value", c->body.value);
                        {
                            value_node->mark_hidden();
                            new_dom_node->values.push_back(std::move(value_node));
                        }

                        new_dom_node->parent = shared_from_this();
                        children[id] = new_dom_node;
                    }
                    else
                    {
                        child->seen = true;
                    }
                }
                c = c->body.next;
            }
        }

        // clang-format off
        iterate_typed_children([this](WorldSettingDomNode *node)
        { 
            auto remove = !node->seen;
            return remove;
        });
        // clang-format on
    }
}