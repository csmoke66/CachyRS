#include "rml_ui.h"

#include <format>
#include <iostream>

#include "log.h"
#include "util.h"
#include "version.hpp"

#include <RmlUi_Platform_SDL.h>
#include <SDL2/SDL.h>

namespace crs
{
  bool CachySystemInterface::LogMessage(Rml::Log::Type, const Rml::String &message)
  {
    LOG(UI, message);
    return true;
  }

  RmlUserInterface::RmlUserInterface()
  {
  }

  RmlUserInterface::~RmlUserInterface()
  {
    if (context)
    {
      Rml::Shutdown();
      Backend::Shutdown();
    }
  }

  void RmlUserInterface::load_fonts()
  {
    struct FontFace
    {
      const char *filename;
      bool fallback_face;
    };

    const Rml::String directory = config_folder + "rmlui/assets/";
    FontFace font_faces[] = {
      { "RobotoMono-Regular.ttf", true },
    };

    for (const FontFace &face : font_faces)
    {
      Rml::LoadFontFace(directory + face.filename, face.fallback_face);
    }
  }

  void RmlUserInterface::pre_init()
  {
    root_dom_node = std::make_shared<DomNode>(shared_from_this(), "root", "root");
  }

  void RmlUserInterface::init(const std::string &app_version, const std::string &config_dir, SDL_Window *window, int width, int height)
  {
    this->version = app_version;
    this->config_folder = config_dir;
    this->sdl_window = window;

    if (!Backend::Initialize(window, width, height))
    {
      return;
    }

    Rml::SetSystemInterface(&system_interface);
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    Rml::Initialise();

    context = Rml::CreateContext("main", Rml::Vector2i(width, height));
    if (!context)
    {
      Rml::Shutdown();
      Backend::Shutdown();
      return;
    }

    visibility_tracker_instancer = std::make_unique<VisibilityTrackerInstancer>();
    visibility_tracker_instancer->parent = this;
    Rml::Factory::RegisterDecoratorInstancer("screen-tracker", visibility_tracker_instancer.get());

    Rml::Debugger::Initialise(context);
    load_fonts();
    reload();
  }

  Rml::ElementDocument *RmlUserInterface::load_document(const std::string &path)
  {
    auto document = context->LoadDocument(path);
    if (document)
    {
      auto title = document->GetElementById("title-bar");

      auto listener = new DragWindowEventListener(title, document);
      title->AddEventListener(Rml::EventId::Dragstart, listener);
      title->AddEventListener(Rml::EventId::Drag, listener);
    }

    return document;
  }

  void RmlUserInterface::reload()
  {
    if (root_document)
    {
      root_document->ReloadStyleSheet();
      root_document->Close();
      root_document = nullptr;

      selected_tab_button = nullptr;
      selected_content = nullptr;

      selected_plugin_tab_button = nullptr;
      selected_plugin_content = nullptr;

      home_tab_button = nullptr;
      home_content = nullptr;

      plugins_tab_button = nullptr;
      plugins_content = nullptr;
      plugins_buttons = nullptr;

      debug_tab_button = nullptr;
      debug_content = nullptr;

      dom_inspector_content = nullptr;

      last_hovered = nullptr;
    }

    for (auto &[key, value] : dom_nodes)
    {
      key->is_built = false;
      key->dirty = true;
    }

    dom_nodes.clear();

    for (auto &[id, document] : document_map)
    {
      document->Close();
    }
    document_map.clear();
    component_map.clear();

    root_document = load_document(config_folder + "rmlui/main.html");
    if (root_document)
    {
      auto *title = root_document->GetElementById("title-version");
      title->SetInnerRML(std::format("{} ({})", FEATURE_VERSION, CACHYRS_VERSION));

      home_tab_button = root_document->GetElementById("home_tab_button");
      home_content = root_document->GetElementById("home_content");

      plugins_tab_button = root_document->GetElementById("plugins_tab_button");
      plugins_content = root_document->GetElementById("plugins_content");
      plugins_buttons = plugins_content->GetElementById("plugins_buttons");

      debug_tab_button = root_document->GetElementById("debug_tab_button");
      debug_content = root_document->GetElementById("debug_content");
      dom_inspector_content = debug_content->GetElementById("dom-inspector");

      home_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                 &selected_tab_button, &selected_content,
                                                                 home_tab_button, home_content));

      plugins_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                    &selected_tab_button, &selected_content,
                                                                    plugins_tab_button, plugins_content));

      debug_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                  &selected_tab_button, &selected_content,
                                                                  debug_tab_button, debug_content));

      auto dom_node_ext = get_rml_dom_node(root_dom_node.get());
      dom_node_ext->element = debug_content->GetElementById("dom-tree");

      auto verify_button = root_document->GetElementById("verify_button");
      verify_button->AddEventListener(Rml::EventId::Click, new VerifyEventHandler(this));

      auto refresh_button = root_document->GetElementById("refresh_button");
      refresh_button->AddEventListener(Rml::EventId::Click, new RefreshEventHandler(this));

      root_document->Show();
    }

    for (auto &f : reload_callbacks)
    {
      f();
    }
  }

  void RmlUserInterface::add_reload_callback(std::function<void()> function)
  {
    reload_callbacks.push_back(std::move(function));
  }

  void RmlUserInterface::release_keyboard_focus()
  {
    overlay_has_keyboard_focus = false;

    if (!context)
    {
      return;
    }

    if (auto *focused = context->GetFocusElement())
    {
      if (auto *document = focused->GetOwnerDocument())
      {
        document->Focus();
      }
      else
      {
        focused->Blur();
      }
    }
  }

  void RmlUserInterface::process(SDL_Event *event)
  {
    const bool is_key = event->type == SDL_KEYDOWN || event->type == SDL_KEYUP || event->type == SDL_TEXTINPUT;
    if (is_key && !overlay_has_keyboard_focus)
    {
      wants_input_last.store(false, std::memory_order_relaxed);
      return;
    }

    wants_input_last.store(!Backend::ProcessEvents(context, event), std::memory_order_relaxed);

    if (event->type == SDL_MOUSEBUTTONDOWN && context)
    {
      if (context->IsMouseInteracting())
      {
        overlay_has_keyboard_focus = true;
      }
      else
      {
        release_keyboard_focus();
      }
    }
  }

  bool RmlUserInterface::wants_input()
  {
    return wants_input_last.load(std::memory_order_relaxed);
  }

  RmlDomNode *RmlUserInterface::get_rml_dom_node(DomNode *node)
  {
    auto [it, inserted] = dom_nodes.try_emplace(node);
    return &it->second;
  }

  RmlDomNode *RmlUserInterface::find_rml_dom_node(DomNode *node)
  {
    auto it = dom_nodes.find(node);
    if (it == dom_nodes.end())
    {
      return nullptr;
    }

    return &it->second;
  }

  RmlComponent *RmlUserInterface::find_component(uint64_t component_id)
  {
    auto it = component_map.find(component_id);
    return it == component_map.end() ? nullptr : &it->second;
  }

  static bool element_display_none(Rml::Element *element)
  {
    if (!element)
    {
      return false;
    }

    auto property = element->GetLocalProperty(Rml::PropertyId::Display);
    if (!property)
    {
      return false;
    }

    return property->Get<Rml::String>() == "none";
  }

  static bool element_below_viewport(Rml::Element *element)
  {
    if (!element)
    {
      return false;
    }

    auto context = element->GetContext();
    if (!context)
    {
      return false;
    }

    auto pos = element->GetAbsoluteOffset(Rml::BoxArea::Border);
    return pos.y > static_cast<float>(context->GetDimensions().y);
  }

  void RmlUserInterface::set_listener(std::unique_ptr<DomTreeListener> listener)
  {
    dom_tree_listener = std::move(listener);
  }

  bool RmlUserInterface::build_dom_node(DomNode *node, int depth)
  {
    auto dom_node_ext = get_rml_dom_node(node);
    auto rendered = !dom_node_ext->wrapper_element || is_rendered(dom_node_ext->wrapper_element);
    node->visible = rendered;

    if (!rendered)
    {
      return false;
    }

    if (!node->is_built)
    {
      auto element = root_document->CreateElement("div");
      element->SetProperty("decorator", "screen-tracker");

      std::string inner_rml;
      inner_rml += std::format("<div><span class=\"dom-node\">&lt;</span><span class=\"dom-node-type\">{}</span>", node->type);

      std::vector<DomValue *> to_render;
      for (auto &value : node->values)
      {
        if (!value->hidden)
        {
          to_render.push_back(value.get());
        }
      }

      if (!to_render.empty())
      {
        inner_rml += "<span>&nbsp;</span>";
      }

      for (size_t i = 0; i < to_render.size(); i++)
      {
        inner_rml += std::format("<span class=\"dom-node-key\">{}</span><span class=\"dom-node\">=</span><span class=\"dom-node-value\" id=\"{}\">&quot;{}&quot;</span>", to_render[i]->name, to_render[i]->id, to_render[i]->to_string());
        if (i + 1 != to_render.size())
        {
          inner_rml += "<span>&nbsp;</span>";
        }
      }
      inner_rml += std::format("<span class=\"dom-node\" id=\"{}\">&gt;</span></div>", node->id);
      inner_rml += std::format("<div><span class=\"dom-node\">&lt;/</span><span class=\"dom-node-type\">{}</span><span class=\"dom-node\">&gt;</span></div>", node->type);

      element->SetInnerRML(inner_rml);
      element->SetClass("dom-row", true);

      auto anchor = element->GetElementById(node->id);
      auto parent_node_ext = get_rml_dom_node(node->parent.get());

      for (auto &value : node->values)
      {
        dom_node_ext->dom_values[value->id] = { element->GetElementById(value->id) };
      }

      dom_node_ext->wrapper_element = parent_node_ext->element->AppendChild(std::move(element));
      dom_node_ext->element = anchor;

      auto dom_node_event_listener = new DomNodeEventListener(this, node->shared_from_this());
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Click, dom_node_event_listener);
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Mouseover, dom_node_event_listener);
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Mouseout, dom_node_event_listener);
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Keydown, dom_node_event_listener);
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Keyup, dom_node_event_listener);
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Dblclick, dom_node_event_listener);
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Dblclick, new ToggleDomNodeEventListener(dom_node_ext->element));

      node->is_built = true;
      node->dirty = false;
    }
    else if (node->dirty)
    {
      for (auto &value : node->values)
      {
        if (value->hidden || !value->dirty)
        {
          continue;
        }

        auto dom_value = dom_node_ext->dom_values.find(value->id);
        if (dom_value != dom_node_ext->dom_values.end() && dom_value->second.element)
        {
          dom_value->second.element->SetInnerRML(std::format("&quot;{}&quot;", value->to_string()));
        }

        value->dirty = false;
      }

      node->dirty = false;
    }

    if (element_display_none(dom_node_ext->element))
    {
      return true;
    }

    auto below_fold = false;
    auto built_new = 0;
    constexpr auto max_new_rows = 64;
    for (auto &child : node->child_order)
    {
      if (auto child_ext = find_rml_dom_node(child.get()))
      {
        if (child->is_built && child_ext->wrapper_element && !is_rendered(child_ext->wrapper_element))
        {
          child->visible = false;
          if (element_below_viewport(child_ext->wrapper_element))
          {
            below_fold = true;
          }
          continue;
        }
      }

      if (!child->is_built && (below_fold || built_new >= max_new_rows))
      {
        continue;
      }

      auto was_built = child->is_built;
      build_dom_node(child.get(), depth + 1);
      if (!was_built && child->is_built)
      {
        built_new += 1;
      }

      if (auto child_ext = find_rml_dom_node(child.get()))
      {
        if (child_ext->wrapper_element && element_below_viewport(child_ext->wrapper_element))
        {
          below_fold = true;
        }
      }
    }

    return true;
  }

  void RmlUserInterface::add_dom_node(std::shared_ptr<DomNode> node)
  {
    root_dom_node->add_child(node);
  }

  void RmlUserInterface::detach_dom_node(DomNode *node, bool remove_element)
  {
    for (auto &child : node->child_order)
    {
      detach_dom_node(child.get(), false);
    }

    auto it = dom_nodes.find(node);
    if (it != dom_nodes.end())
    {
      if (remove_element && it->second.wrapper_element)
      {
        if (auto parent = it->second.wrapper_element->GetParentNode())
        {
          parent->RemoveChild(it->second.wrapper_element);
        }
      }

      dom_nodes.erase(it);
    }

    node->is_built = false;
    node->dirty = true;
  }

  void RmlUserInterface::remove_dom_node(DomNode *node)
  {
    detach_dom_node(node, true);
  }

  void RmlUserInterface::inspect_dom_node(std::shared_ptr<DomNode> node)
  {
    while (dom_inspector_content->HasChildNodes())
    {
      dom_inspector_content->RemoveChild(dom_inspector_content->GetLastChild());
    }

    for (auto &value : node->values)
    {
      auto element = root_document->CreateElement("div");
      element->SetInnerRML(std::format("<div class=\"dom-inspector-entry\"><div class=\"dom-inspector-key\">{}</div><div>{}</div></div>", value->name, value->to_string()));
      dom_inspector_content->AppendChild(std::move(element));
    }
  }

  Rml::Element *RmlUserInterface::get_dom_parent(Rml::Element *element)
  {
    if (!element)
    {
      return nullptr;
    }

    if (element->IsClassSet("dom-row"))
    {
      return element;
    }

    return get_dom_parent(element->GetParentNode());
  }

  bool RmlUserInterface::is_rendered(Rml::Element *element)
  {
    auto rendered = false;
    if (element)
    {
      if (element->HasAttribute("render_frame"))
      {
        auto frame = element->GetAttribute("render_frame")->Get<uint64_t>();
        rendered = (frame == render_frame);
      }
    }
    return rendered;
  }

  void RmlUserInterface::render()
  {
    render_frame += 1;

    auto hovered = get_dom_parent(context->GetHoverElement());
    if (hovered != last_hovered)
    {
      if (last_hovered)
      {
        last_hovered->SetClass("hovered", false);
      }

      last_hovered = hovered;

      if (last_hovered)
      {
        last_hovered->SetClass("hovered", true);
      }
    }

    context->Update();

    Backend::BeginFrame();
    context->Render();
    Backend::PresentFrame();
  }

  uint64_t RmlUserInterface::get_render_frame()
  {
    return render_frame;
  }

  uint64_t RmlUserInterface::allocate_tab(const std::string &name)
  {
    auto id = component_allocation++;

    auto container_el = root_document->CreateElement("div");
    container_el->SetClass("sidebar-content", true);
    container_el->SetProperty("display", "none");
    auto container = plugins_content->AppendChild(std::move(container_el));

    auto button_el = root_document->CreateElement("button");
    button_el->SetInnerRML(name);
    button_el->SetClass("sidebar-tab-button", true);
    auto button = plugins_buttons->AppendChild(std::move(button_el));

    button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                      &selected_plugin_tab_button, &selected_plugin_content,
                                                      button, container));

    component_map[id] = RmlComponent{ .type = ComponentType::container, .element = container, .dropdown = {} };
    return id;
  }

  uint64_t RmlUserInterface::allocate_component(ComponentType type, uint64_t parent_id)
  {
    auto id = component_allocation++;
    auto *parent = find_component(parent_id);
    auto *parent_element = parent ? parent->element : nullptr;
    if (!parent_element)
    {
      LOG(ERROR, "Cannot allocate component with no parent");
      return 0;
    }

    auto *document = parent_element->GetOwnerDocument();
    RmlComponent component;
    auto append = [&](Rml::ElementPtr created)
    {
      component.element = parent_element->AppendChild(std::move(created));
    };

    if (type == ComponentType::container)
    {
      append(document->CreateElement("div"));
    }
    else if (type == ComponentType::label)
    {
      auto element = document->CreateElement("div");
      element->SetInnerRML(std::format("<span id=\"text_{}\">N/A</span>", id));
      append(std::move(element));
    }
    else if (type == ComponentType::hr)
    {
      auto element = document->CreateElement("div");
      element->SetClass("hr-custom", true);
      append(std::move(element));
    }
    else if (type == ComponentType::line)
    {
      auto element = document->CreateElement("div");
      element->SetClass("hr-empty", true);
      append(std::move(element));
    }
    else if (type == ComponentType::button)
    {
      auto element = document->CreateElement("button");
      element->SetId(std::format("{}", id));
      element->SetInnerRML("N/A");
      append(std::move(element));
    }
    else if (type == ComponentType::checkbox)
    {
      auto element = document->CreateElement("div");
      element->SetInnerRML(std::format("<label class=\"checkbox-row\"><input type=\"checkbox\" id=\"input_{}\"/><span id=\"text_{}\">N/A</span></label>", id, id));
      append(std::move(element));
    }
    else if (type == ComponentType::dropdown)
    {
      auto element = document->CreateElement("select");
      element->SetId(std::format("{}", id));
      element->AddEventListener(Rml::EventId::Change, new DropDownChangedEventListener(this, id));
      append(std::move(element));
      component.dropdown.change_handlers = std::vector<std::function<void(int)>>();
    }

    if (!component.element)
    {
      return 0;
    }

    component_map[id] = component;
    return id;
  }

  void RmlUserInterface::update_component_text(uint64_t component_id, const std::string &text)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    auto *element = component->element;
    LOG(DEBUG, "Update text " << component_id << " " << text << " " << element->GetInnerRML());

    auto *child = element->GetElementById(std::format("text_{}", component_id));
    if (!child)
    {
      child = element->GetElementById(std::format("title_{}", component_id));
    }

    if (child)
    {
      child->SetInnerRML(text);
    }
  }

  void RmlUserInterface::update_component_items(uint64_t component_id, const std::vector<std::string> &items)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(component->element);
    if (!select)
    {
      LOG(ERROR, "Invalid element: " << component->element);
      return;
    }

    LOG(DEBUG, "Removing children");
    select->RemoveAll();

    LOG(DEBUG, "Adding options");
    auto idx = 0;
    for (auto &item : items)
    {
      LOG(DEBUG, item);
      select->Add(item, std::format("{}", idx++));
    }
  }

  bool RmlUserInterface::is_component_active(uint64_t component_id)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return false;
    }

    auto *child = component->element->GetElementById(std::format("input_{}", component_id));
    return child && child->HasAttribute("checked");
  }

  void RmlUserInterface::set_component_active(uint64_t component_id, bool active)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    auto *child = component->element->GetElementById(std::format("input_{}", component_id));
    if (!child)
    {
      return;
    }

    if (active)
    {
      child->SetAttribute("checked", true);
    }
    else
    {
      child->RemoveAttribute("checked");
    }
  }

  void RmlUserInterface::register_dropdown_change_handler(uint64_t component_id, std::function<void(int32_t)> handler)
  {
    if (auto *component = find_component(component_id))
    {
      component->dropdown.change_handlers.push_back(handler);
    }
  }

  void RmlUserInterface::dropdown_set_selected(uint64_t component_id, int32_t index)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    auto *select = dynamic_cast<Rml::ElementFormControlSelect *>(component->element);
    if (select)
    {
      select->SetSelection(index);
    }
  }

  void RmlUserInterface::on_dropdown_component_changed(uint64_t component_id, int32_t index)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    for (auto &handler : component->dropdown.change_handlers)
    {
      handler(index);
    }
  }

  void RmlUserInterface::set_component_visible(uint64_t component_id, bool visible)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    if (visible)
    {
      component->element->RemoveProperty("display");
    }
    else
    {
      component->element->SetProperty("display", "none");
    }
  }
} // namespace crs
