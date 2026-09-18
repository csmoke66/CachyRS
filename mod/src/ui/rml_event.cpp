#include "rml_ui.h"

namespace crs
{
  SwitchTabEventHandler::SwitchTabEventHandler(
      Rml::Element **current_tab,
      Rml::Element **current_content,
      Rml::Element *tab,
      Rml::Element *content) : current_tab(current_tab),
                               current_content(current_content),
                               tab(tab),
                               content(content)
  {
  }

  void SwitchTabEventHandler::ProcessEvent(Rml::Event &)
  {
    if (auto current = *current_tab)
    {
      current->SetClass("selected", false);
    }

    if (auto current = *current_content)
    {
      current->SetProperty("display", "none");
    }

    tab->SetClass("selected", true);
    content->RemoveProperty("display");

    *current_tab = tab;
    *current_content = content;
  }

  VerifyEventHandler::VerifyEventHandler(RmlUserInterface *rml_ui) : rml_ui(rml_ui)
  {
  }

  void VerifyEventHandler::ProcessEvent(Rml::Event &)
  {
    rml_ui->request_verify();
  }

  RefreshEventHandler::RefreshEventHandler(RmlUserInterface *rml_ui) : rml_ui(rml_ui)
  {
  }

  void RefreshEventHandler::ProcessEvent(Rml::Event &)
  {
    rml_ui->reload();
  }

  ToggleFeatureEventListener::ToggleFeatureEventListener(bool *val) : val(val)
  {
  }

  void ToggleFeatureEventListener::ProcessEvent(Rml::Event &event)
  {
    auto *checkbox = event.GetCurrentElement();
    *val = checkbox->HasAttribute("checked");
  }

  DomNodeEventListener::DomNodeEventListener(RmlUserInterface *parent, std::shared_ptr<DomNode> node) : parent(parent),
                                                                                                        node(std::move(node))
  {
  }

  void DomNodeEventListener::ProcessEvent(Rml::Event &event)
  {
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

  ToggleDomNodeEventListener::ToggleDomNodeEventListener(Rml::Element *element) : element(element)
  {
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

  DragWindowEventListener::DragWindowEventListener(Rml::Element *, Rml::Element *window) : window(window)
  {
  }

  void DragWindowEventListener::ProcessEvent(Rml::Event &event)
  {
    if (event.GetId() == Rml::EventId::Dragstart)
    {
      int mouse_x = event.GetParameter<int>("mouse_x", 0);
      int mouse_y = event.GetParameter<int>("mouse_y", 0);

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

  DropDownChangedEventListener::DropDownChangedEventListener(RmlUserInterface *parent, uint64_t component_id) : parent(parent),
                                                                                                                component_id(component_id)
  {
  }

  void DropDownChangedEventListener::ProcessEvent(Rml::Event &event)
  {
    auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(event.GetTargetElement());
    if (!select)
    {
      return;
    }

    parent->on_dropdown_component_changed(component_id, select->GetSelection());
  }

  ButtonClickEventListener::ButtonClickEventListener(RmlUserInterface *parent, uint64_t component_id) : parent(parent),
                                                                                                        component_id(component_id)
  {
  }

  void ButtonClickEventListener::ProcessEvent(Rml::Event &event)
  {
    event.StopPropagation();
    parent->on_button_component_clicked(component_id);
  }

  GraphMapEventListener::GraphMapEventListener(RmlUserInterface *parent, uint64_t component_id) : parent(parent),
                                                                                                  component_id(component_id)
  {
  }

  void GraphMapEventListener::ProcessEvent(Rml::Event &event)
  {
    parent->on_graph_map_event(component_id, event);
  }

  GraphMapContextButtonListener::GraphMapContextButtonListener(RmlUserInterface *parent, uint64_t component_id, uint32_t action)
      : parent(parent),
        component_id(component_id),
        action(action)
  {
  }

  void GraphMapContextButtonListener::ProcessEvent(Rml::Event &event)
  {
    event.StopPropagation();
    parent->on_graph_map_context_action(component_id, action);
  }
} // namespace crs