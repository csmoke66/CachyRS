#pragma once
#include "dom.h"
#include "reversed/reversed.h"

namespace crs
{
    class NpcDomNode : public ValueDomNode<Entity*>
    {
    public:
        Entity *npc;
        bool seen = true;

    public:
        NpcDomNode(const ::std::string& id, const ::std::string& type);
    };

    class NpcsDomNode : public TypedChildrenDomNode<NpcDomNode>,
                        public std::enable_shared_from_this<NpcsDomNode>
    {
    public:
        NpcsDomNode(const ::std::string& id, const ::std::string& type);

    public:
        void update();
    };

    class ItemDomNode : public ValueDomNode<Item>
    {
    public:
        Item item;
        bool seen = true;

    public:
        ItemDomNode(const ::std::string& id, const ::std::string& type);
    };

    class ItemContainerDomNode : public TypedChildrenDomNode<ItemDomNode>,
                                  public std::enable_shared_from_this<ItemContainerDomNode>
    {
    public:
        uint32_t id;
        bool seen = true;

    public:
        ItemContainerDomNode(const ::std::string& id, const ::std::string& type);

    public:
        void update(const std::string& parent_id, const ItemContainer& container);
    };

    class ItemContainersDomNode : public TypedChildrenDomNode<ItemContainerDomNode>,
                                  public std::enable_shared_from_this<ItemContainersDomNode>
    {
    public:
        ItemContainersDomNode(const ::std::string& id, const ::std::string& type);

    public:
        void update();
    };
}