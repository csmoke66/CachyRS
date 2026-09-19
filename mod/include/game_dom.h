#pragma once
#include "dom.h"
#include "reversed/reversed.h"

namespace crs
{
  template <typename T>
  class GameContainerNode : public TypedChildrenDomNode<T>
  {
  public:
    GameContainerNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : TypedChildrenDomNode<T>(tree, id, type)
    {
    }

  public:
    virtual void update() = 0;
  };

  class PlayerDomNode : public ValueDomNode<const Entity *>
  {
  public:
    PlayerDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class PlayersDomNode : public GameContainerNode<PlayerDomNode>
  {
  public:
    PlayersDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };

  class NpcDomNode : public ValueDomNode<const Entity *>
  {
  public:
    NpcDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class NpcsDomNode : public GameContainerNode<NpcDomNode>
  {
  public:
    NpcsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };

  class ObjectDomNode : public ValueDomNode<const Entity *>
  {
  public:
    ObjectDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class ObjectsDomNode : public GameContainerNode<ObjectDomNode>
  {
  public:
    ObjectsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };

  class ItemDomNode : public ValueDomNode<Item>
  {
  public:
    ItemDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class ItemContainerDomNode : public TypedChildrenDomNode<ItemDomNode>
  {
  public:
    ItemContainerDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update(const std::string &parent_id, const ItemContainer &container);
  };

  class ItemContainersDomNode : public GameContainerNode<ItemContainerDomNode>
  {
  public:
    ItemContainersDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };

  class WorldSettingDomNode : public DomNode
  {
  public:
    UInt32DomValue *value = nullptr;

  public:
    WorldSettingDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class WorldSettingsDomNode : public GameContainerNode<WorldSettingDomNode>
  {
  public:
    WorldSettingsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };

  class StatDomNode : public DomNode
  {
  public:
    StatDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class StatsDomNode : public GameContainerNode<StatDomNode>
  {
  public:
    StatsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };

  class HoveredWidgetDomNode : public DomNode
  {
  public:
    HoveredWidgetDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);
  };

  class HoveredWidgetsDomNode : public GameContainerNode<HoveredWidgetDomNode>
  {
  public:
    HoveredWidgetsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type);

  public:
    void update() override;
  };
} // namespace crs
