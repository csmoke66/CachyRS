#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include <memory>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

namespace crs
{
    class DomValue
    {
    public:
        ::std::string name;

    public:
        bool dirty = true;
        bool hidden = false;
        bool seen = true;
        
    public:
        DomValue(const ::std::string &name);
        virtual ~DomValue()
        {
        }

    public:
        void mark_dirty();
        void mark_hidden();

    public:
        virtual ::std::string to_string() = 0;
    };

    class Int32DomValue : public DomValue
    {
    public:
        int32_t val;

    public:
        Int32DomValue(const ::std::string &name, int32_t val);

    public:
        ::std::string to_string() override;
    };

    class UInt32DomValue : public DomValue
    {
    public:
        uint32_t val;

    public:
        UInt32DomValue(const ::std::string &name, uint32_t val);

    public:
        ::std::string to_string() override;
    };

    class UInt64DomValue : public DomValue
    {
    public:
        uint64_t val;

    public:
        UInt64DomValue(const ::std::string &name, uint64_t val);

    public:
        ::std::string to_string() override;
    };

    class FloatDomValue : public DomValue
    {
    public:
        float val;

    public:
        FloatDomValue(const ::std::string &name, float val);

    public:
        ::std::string to_string() override;
    };

    class PointerDomValue : public DomValue
    {
    public:
        const void *val;

    public:
        PointerDomValue(const ::std::string &name, const void *val);

    public:
        ::std::string to_string() override;
    };

    class StringDomValue : public DomValue
    {
    public:
        std::string val;

    public:
        StringDomValue(const ::std::string &name, const ::std::string &val);

    public:
        ::std::string to_string() override;
    };

    typedef void (*FnDomFunction)(void *context);
    class FunctionDomValue : public DomValue
    {
    public:
        std::string documentation;
        std::string ret;
        FnDomFunction val;

    public:
        FunctionDomValue(const ::std::string &name, const ::std::string &documentation, const ::std::string &ret, FnDomFunction val);

    public:
        ::std::string to_string() override;
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
        virtual void build_dom_node(std::shared_ptr<DomNode> node, int depth = 0) = 0;
        virtual void add_dom_node(std::shared_ptr<DomNode> node) = 0;
        virtual void remove_dom_node(std::shared_ptr<DomNode> node) = 0;
    };

    class DomNode
    {
    public:
        std::shared_ptr<DomTree> tree;
        ::std::string id;
        ::std::string type;
        ::std::vector<std::unique_ptr<DomValue>> values;

        bool deleted = false;

    public:
        ::std::map<::std::string, std::shared_ptr<DomNode>> children;

    public:
        bool is_built = false;
        std::shared_ptr<DomNode> parent = nullptr;

    public:
        bool dirty = true;

    public:
        DomNode(std::shared_ptr<DomTree> tree, ::std::string id, ::std::string type);
        virtual ~DomNode();

    private:
        DomNode *find_dom_node(DomNode *, const std::string &id);

    public:
        DomNode *find_dom_node(const std::string &id);

    public:
        void mark_dirty();
    };

    template <typename T>
    class ValueDomNode : public DomNode
    {
    public:
        T value;

    public:
        ValueDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : DomNode(tree, id, type)
        {
        }
    };

    template <std::derived_from<DomNode> T>
    class TypedChildrenDomNode : public DomNode,
                                 public std::enable_shared_from_this<TypedChildrenDomNode<T>>
    {
    public:
        TypedChildrenDomNode(std::shared_ptr<DomTree> tree, const ::std::string &id, const ::std::string &type) : DomNode(tree, id, type)
        {
        }

    public:
        std::shared_ptr<T> find_typed_child(const ::std::string &id)
        {
            auto f = children.find(id);
            return (f == children.end()) ? nullptr : std::static_pointer_cast<T>(f->second);
        }

        template <typename F>
        void iterate_typed_children(F fn)
        {
            for (auto it = children.begin(); it != children.end(); it++)
            {
                if (fn((T *)it->second.get()))
                {
                    it->second->deleted = true;
                }
            }
        }
    };
}