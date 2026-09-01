#include "rml_ui.h"

namespace crs
{
    SwitchTabEventHandler::SwitchTabEventHandler(
        Rml::Element **current_tab,
        Rml::Element **current_content,
        Rml::Element *tab,
        Rml::Element *content)
    {
        this->current_tab = current_tab;
        this->current_content = current_content;
        this->tab = tab;
        this->content = content;
    }

    void SwitchTabEventHandler::ProcessEvent(Rml::Event &event)
    {
        if (auto ct = *current_tab)
        {
            ct->SetClass("tabbutton-selected", false);
        }

        if (auto cc = *current_content)
        {
            cc->SetProperty("display", "none");
        }

        tab->SetClass("tabbutton-selected", true);
        content->RemoveProperty("display");

        *current_tab = tab;
        *current_content = content;
    }

    RefreshEventHandler::RefreshEventHandler(RmlUserInterface *rml_ui)
    {
        this->rml_ui = rml_ui;
    }

    void RefreshEventHandler::ProcessEvent(Rml::Event &event)
    {
        rml_ui->reload();
    }

    ToggleFeatureEventListener::ToggleFeatureEventListener(bool *val)
    {
        this->val = val;
    }

    void ToggleFeatureEventListener::ProcessEvent(Rml::Event &event)
    {
        auto *checkbox = event.GetCurrentElement();
        *val = checkbox->HasAttribute("checked");
    }

    DomNodeEventListener::DomNodeEventListener(RmlUserInterface *parent, std::shared_ptr<DomNode> node)
    {
        this->parent = parent;
        this->node = node;
    }

    void DomNodeEventListener::ProcessEvent(Rml::Event &event)
    {
        auto rmlui_node = parent->get_rml_dom_node(node);
        auto wrapper = rmlui_node->wrapper_element;
        if (event.GetId() == Rml::EventId::Click)
        {
            event.StopPropagation();

            parent->inspect_dom_node(node);
            if (auto &listener = parent->dom_tree_listener)
            {
                listener->on_click(node);
            }
        }
    }

    ToggleDomNodeEventListener::ToggleDomNodeEventListener(Rml::Element *element)
    {
        this->element = element;
    }

    void ToggleDomNodeEventListener::ProcessEvent(Rml::Event &event)
    {
        event.StopPropagation();

        if (element->GetLocalProperty("display"))
        {
            element->RemoveProperty("display");
        }
        else
        {
            element->SetProperty("display", "none");
        }
    }

    DragWindowEventListener::DragWindowEventListener(Rml::Element *element, Rml::Element *window)
    {
        this->element = element;
        this->window = window;
    }

    void DragWindowEventListener::ProcessEvent(Rml::Event &event)
    {
        if (event.GetId() == Rml::EventId::Dragstart)
        {
            int mouse_x = event.GetParameter<int>("mouse_x", 0);
            int mouse_y = event.GetParameter<int>("mouse_y", 0);

            // Current element position.
            int left = window->GetAbsoluteLeft();
            int top = window->GetAbsoluteTop();

            drag_offset_x = mouse_x - left;
            drag_offset_y = mouse_y - top;
        }
        else if (event.GetId() == Rml::EventId::Drag)
        {
            int mouse_x = event.GetParameter<int>("mouse_x", 0);
            int mouse_y = event.GetParameter<int>("mouse_y", 0);

            window->SetProperty(
                Rml::PropertyId::Left,
                Rml::Property(mouse_x - drag_offset_x, Rml::Unit::PX));

            window->SetProperty(
                Rml::PropertyId::Top,
                Rml::Property(mouse_y - drag_offset_y, Rml::Unit::PX));
        }
    }

}