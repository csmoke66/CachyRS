#pragma once
#include "dom.h"
#include "reversed/reversed.h"

namespace crs
{
    class PlayerDomNode : public ValueDomNode<Entity*>
    {
    public:
        const Entity *player;
        bool seen = true;

    public:
        PlayerDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);
    };

    class PlayersDomNode : public TypedChildrenDomNode<PlayerDomNode>
    {
    public:
        PlayersDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);

    public:
        void update();
        void prune();
    };

    class NpcDomNode : public ValueDomNode<Entity*>
    {
    public:
        const Entity *npc;
        bool seen = true;

    public:
        NpcDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);
    };

    class NpcsDomNode : public TypedChildrenDomNode<NpcDomNode>
    {
    public:
        NpcsDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);

    public:
        void update();
        void prune();
    };

    class ItemDomNode : public ValueDomNode<Item>
    {
    public:
        Item item;
        bool seen = true;

    public:
        ItemDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);
    };

    class ItemContainerDomNode : public TypedChildrenDomNode<ItemDomNode>
    {
    public:
        uint32_t id;
        bool seen = true;

    public:
        ItemContainerDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);

    public:
        void update(const std::string& parent_id, const ItemContainer& container);
    };

    class ItemContainersDomNode : public TypedChildrenDomNode<ItemContainerDomNode>
    {
    public:
        ItemContainersDomNode(std::shared_ptr<DomTree> tree, const ::std::string& id, const ::std::string& type);

    public:
        void update();
        void prune();
    };
}