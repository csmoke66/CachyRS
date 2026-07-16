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

    DomNode::DomNode(std::shared_ptr<DomTree> tree, ::std::string id, ::std::string type)
    {
        this->tree = tree;
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

    void DomNode::mark_dirty()
    {
        dirty = true;
    }
}