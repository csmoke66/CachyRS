#include "dom.h"
#include <algorithm>
#include <format>

namespace crs
{
  DomTreeListener::~DomTreeListener() = default;
  DomTree::~DomTree() = default;

  DomValue::DomValue(const std::string &name) : name(name)
  {
  }

  void DomValue::mark_dirty()
  {
    dirty = true;
  }

  void DomValue::mark_inlined()
  {
    inlined = true;
  }

  Int32DomValue::Int32DomValue(const std::string &name, int32_t val) : DomValue(name),
                                                                       val(val)
  {
  }

  bool Int32DomValue::set(int32_t v)
  {
    if (val == v)
    {
      return false;
    }

    val = v;
    mark_dirty();
    return true;
  }

  std::string Int32DomValue::to_string()
  {
    return std::to_string(val);
  }

  UInt32DomValue::UInt32DomValue(const std::string &name, uint32_t val) : DomValue(name),
                                                                          val(val)
  {
  }

  bool UInt32DomValue::set(uint32_t v)
  {
    if (val == v)
    {
      return false;
    }

    val = v;
    mark_dirty();
    return true;
  }

  std::string UInt32DomValue::to_string()
  {
    return std::to_string(val);
  }

  UInt64DomValue::UInt64DomValue(const std::string &name, uint64_t val) : DomValue(name),
                                                                          val(val)
  {
  }

  bool UInt64DomValue::set(uint64_t v)
  {
    if (val == v)
    {
      return false;
    }

    val = v;
    mark_dirty();
    return true;
  }

  std::string UInt64DomValue::to_string()
  {
    return std::to_string(val);
  }

  FloatDomValue::FloatDomValue(const std::string &name, float val) : DomValue(name),
                                                                     val(val)
  {
  }

  bool FloatDomValue::set(float v)
  {
    if (val == v)
    {
      return false;
    }

    val = v;
    mark_dirty();
    return true;
  }

  std::string FloatDomValue::to_string()
  {
    return std::format("{:.2f}", val);
  }

  PointerDomValue::PointerDomValue(const std::string &name, const void *val) : DomValue(name),
                                                                               val(val)
  {
  }

  bool PointerDomValue::set(const void *v)
  {
    if (val == v)
    {
      return false;
    }

    val = v;
    mark_dirty();
    return true;
  }

  std::string PointerDomValue::to_string()
  {
    return std::format("{}", val);
  }

  StringDomValue::StringDomValue(const std::string &name, const std::string &val) : DomValue(name),
                                                                                    val(val)
  {
  }

  bool StringDomValue::set(const std::string &v)
  {
    if (val == v)
    {
      return false;
    }

    val = v;
    mark_dirty();
    return true;
  }

  std::string StringDomValue::to_string()
  {
    return val;
  }

  FunctionDomValue::FunctionDomValue(const std::string &name, const std::string &documentation, const std::string &ret, FnDomFunction val) : DomValue(name),
                                                                                                                                             documentation(documentation),
                                                                                                                                             ret(ret),
                                                                                                                                             val(val)
  {
  }

  std::string FunctionDomValue::to_string()
  {
    return std::format("{}", reinterpret_cast<void *>(val));
  }

  DomNode::DomNode(std::shared_ptr<DomTree> tree, std::string id, std::string type) : id(std::move(id)),
                                                                                      type(std::move(type)),
                                                                                      tree(std::move(tree))
  {
  }

  DomNode::~DomNode() = default;

  DomNode *DomNode::find_dom_node(DomNode *current, const std::string &node_id)
  {
    if (current->id == node_id)
    {
      return current;
    }

    for (auto &child : current->child_order)
    {
      if (auto node = find_dom_node(child.get(), node_id))
      {
        return node;
      }
    }
    return nullptr;
  }

  DomNode *DomNode::find_dom_node(const std::string &node_id)
  {
    return find_dom_node(this, node_id);
  }

  void DomNode::add_value(std::unique_ptr<DomValue> value)
  {
    value->id = std::format("{}_val_{}", id, sanitize_dom_id(value->name));
    values.push_back(std::move(value));
    values_dirty = true;
    mark_dirty();
  }

  void DomNode::remove_value(const std::string &name)
  {
    auto erased = std::erase_if(values, [&name](const std::unique_ptr<DomValue> &value)
    {
      return value->name == name;
    });
    if (erased)
    {
      values_dirty = true;
      mark_dirty();
    }
  }

  void DomNode::add_child(std::shared_ptr<DomNode> child)
  {
    child->parent = shared_from_this();
    child->seen_gen = sync_gen;
    child->deleted = false;
    auto [it, inserted] = children.emplace(child->id, child);
    if (inserted)
    {
      child_order.push_back(child);
      return;
    }

    auto previous = it->second;
    if (previous == child)
    {
      return;
    }

    it->second = child;

    auto slot = std::find(child_order.begin(), child_order.end(), previous);
    if (slot != child_order.end())
    {
      *slot = child;
    }
    else
    {
      child_order.push_back(child);
    }

    if (previous->keyed)
    {
      auto keyed = children_by_key.find(previous->key);
      if (keyed != children_by_key.end() && keyed->second == previous)
      {
        children_by_key.erase(keyed);
      }
    }

    tree->remove_dom_node(previous.get());
    unlink_subtree(previous.get());
  }

  void DomNode::unlink_subtree(DomNode *node)
  {
    for (auto &child : node->child_order)
    {
      unlink_subtree(child.get());
    }

    node->parent = nullptr;
    node->children_by_key.clear();
    node->children.clear();
    node->child_order.clear();
  }

  void DomNode::add_keyed_child(uintptr_t child_key, std::shared_ptr<DomNode> child)
  {
    child->key = child_key;
    child->keyed = true;
    add_child(child);
    children_by_key[child_key] = std::move(child);
  }

  std::shared_ptr<DomNode> DomNode::touch_keyed(uintptr_t child_key)
  {
    auto it = children_by_key.find(child_key);
    if (it == children_by_key.end())
    {
      return nullptr;
    }

    it->second->seen_gen = sync_gen;
    return it->second;
  }

  void DomNode::begin_sync()
  {
    if (++sync_gen == 0)
    {
      for (auto &child : child_order)
      {
        child->seen_gen = 0;
      }
      sync_gen = 1;
    }
  }

  void DomNode::end_sync()
  {
    needs_prune = false;
    for (auto &child : child_order)
    {
      if (child->seen_gen != sync_gen)
      {
        child->deleted = true;
        needs_prune = true;
      }
    }
  }

  uint32_t DomNode::count_deleted_children() const
  {
    uint32_t count = 0;
    for (auto &child : child_order)
    {
      count += static_cast<uint32_t>(child->deleted);
    }
    return count;
  }

  void DomNode::prune()
  {
    for (auto &child : child_order)
    {
      if (child->needs_prune || !child->child_order.empty())
      {
        child->prune();
      }
    }

    if (!needs_prune)
    {
      return;
    }

    std::erase_if(child_order, [this](const std::shared_ptr<DomNode> &child)
    {
      if (!child->deleted)
      {
        return false;
      }

      if (child->keyed)
      {
        children_by_key.erase(child->key);
      }
      children.erase(child->id);
      tree->remove_dom_node(child.get());
      unlink_subtree(child.get());
      return true;
    });

    needs_prune = false;
  }

  void DomNode::mark_dirty()
  {
    dirty = true;
  }
} // namespace crs
