#include "cachy.h"
#include "dom_sync.h"
#include "game_dom.h"
#include "not_cachy.h"

namespace crs
{
  ObjectDomNode::ObjectDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : ValueDomNode<const Entity *>(tree, id, type)
  {
  }

  ObjectsDomNode::ObjectsDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : GameContainerNode<ObjectDomNode>(tree, id, type)
  {
  }

  void ObjectsDomNode::update()
  {
  
  }
} // namespace crs
