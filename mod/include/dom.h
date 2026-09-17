#pragma once
#include <charconv>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

namespace crs
{
  inline std::string make_numeric_id(std::string_view prefix, uint32_t n)
  {
    std::string out;
    out.reserve(prefix.size() + 11);
    out.append(prefix);
    char buf[16];
    auto r = std::to_chars(buf, buf + sizeof(buf), n);
    out.append(buf, static_cast<size_t>(r.ptr - buf));
    return out;
  }

  inline std::string make_pointer_id(std::string_view prefix, const void *p)
  {
    std::string out;
    out.reserve(prefix.size() + 18);
    out.append(prefix);
    out.append("0x");
    char buf[16];
    auto r = std::to_chars(buf, buf + sizeof(buf), reinterpret_cast<uintptr_t>(p), 16);
    out.append(buf, static_cast<size_t>(r.ptr - buf));
    return out;
  }

  class DomValue
  {
  public:
    std::string name;
    std::string id;

  public:
    bool dirty = true;
    bool hidden = false;

  public:
    DomValue(const std::string &name);
    virtual ~DomValue() = default;

  public:
    void mark_dirty();
    void mark_hidden();

  public:
    virtual std::string to_string() = 0;
  };

  class Int32DomValue : public DomValue
  {
  public:
    int32_t val;

  public:
    Int32DomValue(const std::string &name, int32_t val);

  public:
    bool set(int32_t v);
    std::string to_string() override;
  };

  class UInt32DomValue : public DomValue
  {
  public:
    uint32_t val;

  public:
    UInt32DomValue(const std::string &name, uint32_t val);

  public:
    bool set(uint32_t v);
    std::string to_string() override;
  };

  class UInt64DomValue : public DomValue
  {
  public:
    uint64_t val;

  public:
    UInt64DomValue(const std::string &name, uint64_t val);

  public:
    bool set(uint64_t v);
    std::string to_string() override;
  };

  class FloatDomValue : public DomValue
  {
  public:
    float val;

  public:
    FloatDomValue(const std::string &name, float val);

  public:
    bool set(float v);
    std::string to_string() override;
  };

  class PointerDomValue : public DomValue
  {
  public:
    const void *val;

  public:
    PointerDomValue(const std::string &name, const void *val);

  public:
    bool set(const void *v);
    std::string to_string() override;
  };

  class StringDomValue : public DomValue
  {
  public:
    std::string val;

  public:
    StringDomValue(const std::string &name, const std::string &val);

  public:
    bool set(const std::string &v);
    std::string to_string() override;
  };

  using FnDomFunction = void (*)(void *context);
  class FunctionDomValue : public DomValue
  {
  public:
    std::string documentation;
    std::string ret;
    FnDomFunction val;

  public:
    FunctionDomValue(const std::string &name, const std::string &documentation, const std::string &ret, FnDomFunction val);

  public:
    std::string to_string() override;
  };

  class DomNode;

  class DomTreeListener
  {
  public:
    virtual ~DomTreeListener();

  public:
    virtual void on_click(std::shared_ptr<DomNode> node) = 0;
  };

  class DomTree
  {
  public:
    virtual ~DomTree();

  public:
    virtual void set_listener(std::unique_ptr<DomTreeListener> listener) = 0;

  public:
    virtual bool build_dom_node(DomNode *node, int depth = 0) = 0;
    virtual void add_dom_node(std::shared_ptr<DomNode> node) = 0;
    virtual void remove_dom_node(DomNode *node) = 0;
  };

  class DomNode : public std::enable_shared_from_this<DomNode>
  {
  public:
    // Hot: walked for every child during sync / RML virtualization.
    uint32_t seen_gen = 0;
    uint32_t sync_gen = 0;
    uintptr_t key = 0;
    bool deleted = false;
    bool visible = false;
    bool dirty = true;
    bool is_built = false;
    bool needs_prune = false;
    bool keyed = false;

    std::string id;
    std::string type;
    std::vector<std::unique_ptr<DomValue>> values;

    std::shared_ptr<DomNode> parent = nullptr;
    std::vector<std::shared_ptr<DomNode>> child_order;
    std::unordered_map<std::string, std::shared_ptr<DomNode>> children;
    std::unordered_map<uintptr_t, std::shared_ptr<DomNode>> children_by_key;
    std::shared_ptr<DomTree> tree;

  public:
    DomNode(std::shared_ptr<DomTree> tree, std::string id, std::string type);
    virtual ~DomNode();

  private:
    DomNode *find_dom_node(DomNode *current, const std::string &node_id);

  public:
    DomNode *find_dom_node(const std::string &node_id);

  public:
    void add_value(std::unique_ptr<DomValue> value);
    void add_child(std::shared_ptr<DomNode> child);
    void add_keyed_child(uintptr_t child_key, std::shared_ptr<DomNode> child);
    std::shared_ptr<DomNode> touch_keyed(uintptr_t child_key);

    template <std::derived_from<DomValue> T>
    T *find_value(const std::string &name)
    {
      for (auto &v : values)
      {
        if (v->name == name)
        {
          return static_cast<T *>(v.get());
        }
      }
      return nullptr;
    }

    template <std::derived_from<DomValue> T, typename U>
    bool set_value(const std::string &name, const U &v)
    {
      auto value = find_value<T>(name);
      if (!value)
      {
        return false;
      }

      if (value->set(v))
      {
        if (!value->hidden)
        {
          mark_dirty();
        }
        return true;
      }

      return false;
    }

  public:
    void begin_sync();
    void end_sync();
    uint32_t count_deleted_children() const;
    void prune();
    void mark_dirty();
  };

  template <typename T>
  class ValueDomNode : public DomNode
  {
  public:
    T value;

  public:
    ValueDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : DomNode(tree, id, type)
    {
    }
  };

  template <std::derived_from<DomNode> T>
  class TypedChildrenDomNode : public DomNode
  {
  public:
    TypedChildrenDomNode(std::shared_ptr<DomTree> tree, const std::string &id, const std::string &type) : DomNode(tree, id, type)
    {
    }

  public:
    std::shared_ptr<T> find_typed_child(const std::string &id)
    {
      auto it = children.find(id);
      return (it == children.end()) ? nullptr : std::static_pointer_cast<T>(it->second);
    }

    std::shared_ptr<T> touch_typed_child(const std::string &id)
    {
      auto child = find_typed_child(id);
      if (child)
      {
        child->seen_gen = sync_gen;
      }
      return child;
    }

    std::shared_ptr<T> touch_typed_key(uintptr_t child_key)
    {
      auto child = touch_keyed(child_key);
      return child ? std::static_pointer_cast<T>(child) : nullptr;
    }
  };
} // namespace crs
