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
    struct DomValue
    {
        std::string name;
        std::string value;
    };

    class DomNode
    {
    public:
        ::std::string id;
        ::std::string type;
        ::std::vector<DomValue> values;

    public:
        ::std::map<::std::string, std::shared_ptr<DomNode>> children;

    public:
        bool is_built = false;
        std::shared_ptr<DomNode> parent = nullptr;
        Rml::Element *wrapper_element = nullptr;
        Rml::Element *element = nullptr;

    public:
        DomNode(::std::string id, ::std::string type);

    private:
        DomNode *find_dom_node(DomNode *, const std::string &id);

    public:
        DomNode *find_dom_node(const std::string &id);

    public:
        void destroy_dom_node(DomNode *child);
        void remove_dom_node(const std::string &id);

    public:
        void reset();
    };

    class DomTree
    {
    public:
        ::std::map<std::string, std::shared_ptr<DomNode>> nodes;
    };

    class GameNode
    {
    public:
        virtual void update() = 0;
    };

    template <typename T>
    class ValueDomNode : public DomNode
    {
    public:
        T value;

    public:
        ValueDomNode(const ::std::string &id, const ::std::string &type) : DomNode(id, type)
        {
        }
    };

    template <std::derived_from<DomNode> T>
    class TypedChildrenDomNode : public DomNode
    {
    public:
        TypedChildrenDomNode(const ::std::string& id, const ::std::string& type) : DomNode(id, type)
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
            for (auto it = children.begin(); it != children.end();)
            {
                if (fn((T *)it->second.get()))
                {
                    destroy_dom_node(it->second.get());
                    it = children.erase(it);
                }
                else
                {
                    it++;
                }
            }
        }
    };
}