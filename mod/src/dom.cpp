#include "dom.h"
#include <format>

namespace crs
{
    DomValue::DomValue(const ::std::string &name)
    {
        this->name = name;
    }

    void DomValue::mark_dirty()
    {
        dirty = true;
    }

    void DomValue::mark_hidden()
    {
        hidden = true;
    }

    Int32DomValue::Int32DomValue(const ::std::string &name, int32_t val) : DomValue(name)
    {
        this->val = val;
    }

    ::std::string Int32DomValue::to_string()
    {
        return std::to_string(val);
    }

    UInt32DomValue::UInt32DomValue(const ::std::string &name, uint32_t val) : DomValue(name)
    {
        this->val = val;
    }

    ::std::string UInt32DomValue::to_string()
    {
        return std::to_string(val);
    }

    UInt64DomValue::UInt64DomValue(const ::std::string &name, uint64_t val) : DomValue(name)
    {
        this->val = val;
    }

    ::std::string UInt64DomValue::to_string()
    {
        return std::to_string(val);
    }

    FloatDomValue::FloatDomValue(const ::std::string &name, float val) : DomValue(name) 
    {
        this->val = val;
    }

    ::std::string FloatDomValue::to_string()
    {
        return std::format("{:.2f}", val);
    }

    PointerDomValue::PointerDomValue(const ::std::string &name, const void *val) : DomValue(name)
    {
        this->val = val;
    }

    ::std::string PointerDomValue::to_string()
    {
        return std::string("0x") + std::format("{}", val);
    }

    StringDomValue::StringDomValue(const ::std::string &name, const ::std::string &val) : DomValue(name)
    {
        this->val = val;
    }

    ::std::string StringDomValue::to_string()
    {
        return val;
    }

    FunctionDomValue::FunctionDomValue(const ::std::string &name, const ::std::string &documentation, const ::std::string &ret, FnDomFunction val) : DomValue("name")
    {
        this->documentation = documentation;
        this->ret = ret;
        this->val = val;
    }

    ::std::string FunctionDomValue::to_string()
    {
        return std::string("0x") + std::format("{}", (void *)val);
    }

    DomNode::DomNode(::std::string id, ::std::string type)
    {
        this->id = id;
        this->type = type;
    }

    DomNode *DomNode::find_dom_node(DomNode *current, const std::string &id)
    {
        if (current->id == id)
        {
            return current;
        }

        for (auto child : current->children)
        {
            if (auto node = find_dom_node(child.second.get(), id))
            {
                return node;
            }
        }
        return nullptr;
    }

    DomNode *DomNode::find_dom_node(const std::string &id)
    {
        return find_dom_node(this, id);
    }

    void DomNode::destroy_dom_node(DomNode *child)
    {
        if (child->wrapper_element)
        {
            auto parent = child->wrapper_element->GetParentNode();
            parent->RemoveChild(child->wrapper_element);
        }
    }

    void DomNode::remove_dom_node(const std::string &id)
    {
        auto child = children.find(id);
        if (child != children.end())
        {
            destroy_dom_node(child->second.get());
            children.erase(child);
        }
    }

    void DomNode::reset()
    {
        while (element && element->HasChildNodes())
        {
            element->RemoveChild(element->GetLastChild());
        }
        children.clear();
    }

    void DomNode::mark_dirty()
    {
        dirty = true;
    }
}