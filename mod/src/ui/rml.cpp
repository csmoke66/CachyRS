#include "rml_ui.h"

#include <algorithm>
#include <cmath>
#include <format>
#include <iostream>

#include "log.h"
#include "util.h"

#if __has_include("version.hpp")
#include "version.hpp"
#else
#ifndef FEATURE_VERSION
#define FEATURE_VERSION "0.0.0"
#endif
#ifndef CACHYRS_VERSION
#define CACHYRS_VERSION "sandbox"
#endif
#endif

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

    graph_map_decorator_instancer = std::make_unique<GraphMapDecoratorInstancer>();
    graph_map_decorator_instancer->parent = this;
    Rml::Factory::RegisterDecoratorInstancer("graph-map", graph_map_decorator_instancer.get());

    Rml::Debugger::Initialise(context);
    load_fonts();
    reload();
  }

  Rml::ElementDocument *RmlUserInterface::load_document(const std::string &path)
  {
    auto document = context->LoadDocument(path);
    if (document)
    {
      if (auto title = document->GetElementById("title-bar"))
      {
        auto listener = new DragWindowEventListener(title, document);
        title->AddEventListener(Rml::EventId::Dragstart, listener);
        title->AddEventListener(Rml::EventId::Drag, listener);
      }
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

      logs_tab_button = nullptr;
      logs_content = nullptr;
      log_list = nullptr;
      last_log_row = nullptr;
      log_sync_state = {};
      log_row_index = 0;

      dom_inspector_content = nullptr;
      selected_dom_row = nullptr;
      icon_tooltip = nullptr;
      inspected_dom_node.reset();

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

      logs_tab_button = root_document->GetElementById("logs_tab_button");
      logs_content = root_document->GetElementById("logs_content");
      log_list = logs_content ? logs_content->GetElementById("log-list") : nullptr;
      last_log_row = nullptr;
      log_sync_state = {};
      log_row_index = 0;

      home_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                 &selected_tab_button, &selected_content,
                                                                 home_tab_button, home_content));

      plugins_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                    &selected_tab_button, &selected_content,
                                                                    plugins_tab_button, plugins_content));

      debug_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                  &selected_tab_button, &selected_content,
                                                                  debug_tab_button, debug_content));

      if (logs_tab_button && logs_content)
      {
        logs_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                   &selected_tab_button, &selected_content,
                                                                   logs_tab_button, logs_content));
      }

      if (auto *clear_logs = root_document->GetElementById("logs_clear_button"))
      {
        clear_logs->AddEventListener(Rml::EventId::Click, new ClearLogsEventListener());
      }

      auto dom_node_ext = get_rml_dom_node(root_dom_node.get());
      dom_node_ext->element = debug_content->GetElementById("dom-tree");
      root_dom_node->is_built = true;
      root_dom_node->dirty = false;

      auto verify_button = root_document->GetElementById("verify_button");
      verify_button->AddEventListener(Rml::EventId::Click, new VerifyEventHandler(this));

      auto refresh_button = root_document->GetElementById("refresh_button");
      refresh_button->AddEventListener(Rml::EventId::Click, new RefreshEventHandler(this));

      auto widget_pick_button = root_document->GetElementById("widget_pick_button");
      if (widget_pick_button)
      {
        widget_pick_button->AddEventListener(Rml::EventId::Click, new WidgetPickEventHandler(this, WidgetPickMode::all));
      }

      auto widget_pick_menu_button = root_document->GetElementById("widget_pick_menu_button");
      if (widget_pick_menu_button)
      {
        widget_pick_menu_button->AddEventListener(Rml::EventId::Click, new WidgetPickEventHandler(this, WidgetPickMode::with_menu_options));
      }

      icon_tooltip = root_document->GetElementById("icon_tooltip");
      for (const char *id : { "verify_button", "refresh_button", "widget_pick_button", "widget_pick_menu_button" })
      {
        if (auto *button = root_document->GetElementById(id))
        {
          button->AddEventListener(Rml::EventId::Mouseover, new IconTooltipEventListener(this, true));
          button->AddEventListener(Rml::EventId::Mouseout, new IconTooltipEventListener(this, false));
        }
      }

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

  void RmlUserInterface::set_widget_pick_handler(std::function<void(WidgetPickMode)> handler)
  {
    widget_pick_handler = std::move(handler);
  }

  void RmlUserInterface::on_widget_pick(WidgetPickMode mode)
  {
    if (widget_pick_handler)
    {
      widget_pick_handler(mode);
    }
  }

  void RmlUserInterface::show_main_tab(const std::string &content_id)
  {
    if (!root_document)
    {
      return;
    }

    struct Tab
    {
      Rml::Element *button;
      Rml::Element *content;
      const char *id;
    };

    const Tab tabs[] = {
      { home_tab_button, home_content, "home_content" },
      { plugins_tab_button, plugins_content, "plugins_content" },
      { debug_tab_button, debug_content, "debug_content" },
      { logs_tab_button, logs_content, "logs_content" },
    };

    Rml::Element *want_button = nullptr;
    Rml::Element *want_content = nullptr;
    for (const auto &tab : tabs)
    {
      if (tab.content && content_id == tab.id)
      {
        want_button = tab.button;
        want_content = tab.content;
        break;
      }
    }

    if (!want_content)
    {
      return;
    }

    for (const auto &tab : tabs)
    {
      if (!tab.button || !tab.content)
      {
        continue;
      }

      if (tab.content == want_content)
      {
        tab.button->SetClass("selected", true);
        tab.content->RemoveProperty("display");
      }
      else
      {
        tab.button->SetClass("selected", false);
        tab.content->SetProperty("display", "none");
      }
    }

    selected_tab_button = want_button;
    selected_content = want_content;
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

  bool RmlUserInterface::has_component(uint64_t component_id)
  {
    return find_component(component_id) != nullptr;
  }

  bool RmlUserInterface::component_is_type(uint64_t component_id, ComponentType type)
  {
    auto *component = find_component(component_id);
    return component && component->type == type;
  }

  static bool element_display_none(Rml::Element *element)
  {
    return element && element->GetDisplay() == Rml::Style::Display::None;
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

  static std::vector<DomValue *> visible_dom_values(DomNode *node)
  {
    std::vector<DomValue *> to_render;
    to_render.reserve(node->values.size());
    for (auto &value : node->values)
    {
      if (value->inlined)
      {
        to_render.push_back(value.get());
      }
    }
    return to_render;
  }

  static std::string format_dom_opening_prefix(DomNode *node, const std::vector<DomValue *> &to_render)
  {
    std::string inner_rml;
    inner_rml += std::format(
        "<span class=\"dom-bracket\">&lt;</span><span class=\"dom-tag\">{}</span>",
        escape_rml_text(node->type));

    for (size_t i = 0; i < to_render.size(); i++)
    {
      inner_rml += std::format(
          "<span>&nbsp;</span><span class=\"dom-attr-name\">{}</span><span class=\"dom-attr-eq\">=</span>"
          "<span class=\"dom-attr-value\" id=\"{}\">&quot;{}&quot;</span>",
          escape_rml_text(to_render[i]->name),
          to_render[i]->id,
          escape_rml_text(to_render[i]->to_string()));
    }

    inner_rml += "<span class=\"dom-bracket\">&gt;</span>";
    return inner_rml;
  }

  static std::string format_dom_collapsed_tail(DomNode *node)
  {
    return std::format(
        "<span class=\"dom-ellipsis\">...</span>"
        "<span class=\"dom-bracket\">&lt;/</span><span class=\"dom-tag\">{}</span><span class=\"dom-bracket\">&gt;</span>",
        escape_rml_text(node->type));
  }

  static void bind_dom_value_elements(RmlDomNode *dom_node_ext, DomNode *node, Rml::Element *scope)
  {
    dom_node_ext->dom_values.clear();
    for (auto &value : node->values)
    {
      dom_node_ext->dom_values[value->id] = { scope ? scope->GetElementById(value->id) : nullptr };
      value->dirty = false;
    }
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
      if (!node->parent)
      {
        LOG(ERROR, "Cannot build DOM node without a parent: " << node->id);
        return false;
      }

      auto parent_node_ext = get_rml_dom_node(node->parent.get());
      if (!parent_node_ext->element)
      {
        LOG(ERROR, "Cannot build DOM node with unready parent: " << node->id);
        return false;
      }

      auto to_render = visible_dom_values(node);

      auto element = root_document->CreateElement("div");
      element->SetProperty("decorator", "screen-tracker");
      element->SetClass("dom-row", true);
      element->SetClass("dom-collapsed", true);
      element->SetInnerRML(std::format(
          "<div class=\"dom-line\">"
          "<span class=\"dom-caret\">"
          "<span class=\"dom-caret-plus\">+</span>"
          "<span class=\"dom-caret-minus\">-</span>"
          "</span>"
          "<span class=\"dom-tag-open\">{}</span>"
          "<span class=\"dom-collapsed-tail\">{}</span>"
          "</div>"
          "<div class=\"dom-children\" id=\"{}\"></div>"
          "<div class=\"dom-line dom-close\">"
          "<span class=\"dom-bracket\">&lt;/</span><span class=\"dom-tag\">{}</span><span class=\"dom-bracket\">&gt;</span>"
          "</div>",
          format_dom_opening_prefix(node, to_render),
          format_dom_collapsed_tail(node),
          node->id,
          escape_rml_text(node->type)));

      auto anchor = element->GetElementById(node->id);
      if (!anchor)
      {
        LOG(ERROR, "Failed to create DOM anchor for node: " << node->id);
        return false;
      }

      auto *opening = element->GetChild(0);
      // Visibility of children/close/tail is owned by .dom-collapsed in RCSS.
      bind_dom_value_elements(dom_node_ext, node, opening);

      dom_node_ext->wrapper_element = parent_node_ext->element->AppendChild(std::move(element));
      dom_node_ext->element = anchor;

      auto *listener = new DomNodeEventListener(this, node->shared_from_this());
      dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Click, listener);
      if (opening)
      {
        if (auto *caret = opening->GetChild(0))
        {
          caret->AddEventListener(Rml::EventId::Click, new ToggleDomNodeEventListener(dom_node_ext->wrapper_element));
        }
      }

      node->is_built = true;
      node->dirty = false;
      node->values_dirty = false;
    }
    else if (node->dirty)
    {
      if (node->values_dirty)
      {
        rebuild_dom_node_values(node, dom_node_ext);
        if (auto inspected = inspected_dom_node.lock())
        {
          if (inspected.get() == node)
          {
            inspect_dom_node(inspected);
          }
        }
        node->values_dirty = false;
      }

      for (auto &value : node->values)
      {
        if (!value->inlined || !value->dirty)
        {
          continue;
        }

        auto dom_value = dom_node_ext->dom_values.find(value->id);
        if (dom_value != dom_node_ext->dom_values.end() && dom_value->second.element)
        {
          dom_value->second.element->SetInnerRML(std::format("&quot;{}&quot;", escape_rml_text(value->to_string())));
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

    if (auto inspected = inspected_dom_node.lock())
    {
      if (inspected.get() == node)
      {
        inspected_dom_node.reset();
      }
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
    node->values_dirty = false;
  }

  void RmlUserInterface::remove_dom_node(DomNode *node)
  {
    detach_dom_node(node, true);
  }

  void RmlUserInterface::rebuild_dom_node_values(DomNode *node, RmlDomNode *dom_node_ext)
  {
    auto *wrapper = dom_node_ext->wrapper_element;
    auto *anchor = dom_node_ext->element;
    if (!wrapper || !anchor || wrapper->GetNumChildren() < 1)
    {
      return;
    }

    auto *opening = wrapper->GetChild(0);
    if (!opening || opening->GetNumChildren() < 2)
    {
      LOG(ERROR, "DOM opening tag layout mismatch for node: " << node->id);
      return;
    }

    auto *tag_open = opening->GetChild(1);
    if (!tag_open)
    {
      LOG(ERROR, "DOM opening tag layout mismatch for node: " << node->id);
      return;
    }

    auto to_render = visible_dom_values(node);
    tag_open->SetInnerRML(format_dom_opening_prefix(node, to_render));
    bind_dom_value_elements(dom_node_ext, node, tag_open);
  }

  void RmlUserInterface::set_dom_row_selected(Rml::Element *row)
  {
    if (selected_dom_row == row)
    {
      return;
    }

    if (selected_dom_row)
    {
      selected_dom_row->SetClass("selected", false);
    }

    selected_dom_row = row;
    if (selected_dom_row)
    {
      selected_dom_row->SetClass("selected", true);
    }
  }

  void RmlUserInterface::inspect_dom_node(std::shared_ptr<DomNode> node)
  {
    inspected_dom_node = node;
    if (!dom_inspector_content)
    {
      return;
    }

    while (dom_inspector_content->HasChildNodes())
    {
      dom_inspector_content->RemoveChild(dom_inspector_content->GetLastChild());
    }

    for (auto &value : node->values)
    {
      auto text = value->to_string();
      if (text.empty())
      {
        continue;
      }

      auto element = root_document->CreateElement("div");
      element->SetClass("dom-prop-row", true);
      element->SetInnerRML(std::format(
          "<span class=\"dom-prop-name\">{}</span>"
          "<span class=\"dom-prop-sep\">:&nbsp;</span>"
          "<span class=\"dom-prop-value\">{}</span>",
          escape_rml_text(value->name),
          escape_rml_text(text)));
      dom_inspector_content->AppendChild(std::move(element));
    }
  }

  static const char *log_level_class(const std::string &level)
  {
    if (level == "ERROR")
    {
      return "log-level-ERROR";
    }
    if (level == "WARNING")
    {
      return "log-level-WARNING";
    }
    if (level == "DEBUG")
    {
      return "log-level-DEBUG";
    }
    if (level == "PLUGIN")
    {
      return "log-level-PLUGIN";
    }
    if (level == "UI")
    {
      return "log-level-UI";
    }
    if (level == "INFO")
    {
      return "log-level-INFO";
    }
    return "log-level-INFO";
  }

  void RmlUserInterface::append_log_row(const UiLogEntry &entry)
  {
    if (!log_list || !root_document)
    {
      return;
    }

    auto row = root_document->CreateElement("div");
    row->SetClass("log-row", true);
    row->SetClass(log_level_class(entry.level), true);
    if ((log_row_index++ % 2) == 1)
    {
      row->SetClass("log-row-alt", true);
    }

    const auto count_style = entry.count > 1 ? "" : " style=\"display:none;\"";
    row->SetInnerRML(std::format(
        "<span class=\"log-level\">{}</span>"
        "<span class=\"log-msg\">{}</span>"
        "<span class=\"log-count\"{}>{}</span>",
        escape_rml_text(entry.level),
        escape_rml_text(entry.message),
        count_style,
        entry.count));

    last_log_row = log_list->AppendChild(std::move(row));
  }

  void RmlUserInterface::update_last_log_count(uint32_t count)
  {
    if (!last_log_row || count < 1)
    {
      return;
    }

    if (auto *bubble = last_log_row->GetChild(last_log_row->GetNumChildren() - 1))
    {
      if (count <= 1)
      {
        bubble->SetProperty("display", "none");
      }
      else
      {
        bubble->RemoveProperty("display");
        bubble->SetInnerRML(std::to_string(count));
      }
    }
  }

  void RmlUserInterface::sync_log_view()
  {
    if (!log_list)
    {
      return;
    }

    std::vector<UiLogEntry> neu;
    uint32_t last_count = 0;
    bool reset = false;
    ui_log_sync(log_sync_state, neu, last_count, reset);

    if (reset)
    {
      while (log_list->HasChildNodes())
      {
        log_list->RemoveChild(log_list->GetLastChild());
      }
      last_log_row = nullptr;
      log_row_index = 0;
    }

    for (const auto &entry : neu)
    {
      append_log_row(entry);
    }

    if (!reset && last_count > 0)
    {
      update_last_log_count(last_count);
    }

    if (reset || !neu.empty())
    {
      log_list->SetScrollTop(log_list->GetScrollHeight());
    }
  }

  void RmlUserInterface::show_icon_tooltip(Rml::Element *anchor, const Rml::String &text)
  {
    if (!icon_tooltip || !anchor || text.empty())
    {
      return;
    }

    auto *window = root_document ? root_document->GetElementById("main_window") : nullptr;
    if (!window)
    {
      return;
    }

    // Avoid Context::Update() here — it re-enters layout during event dispatch and can crash.
    icon_tooltip->SetInnerRML(escape_rml_text(text));
    icon_tooltip->RemoveProperty(Rml::PropertyId::Width);

    const auto window_pos = window->GetAbsoluteOffset(Rml::BoxArea::Border);
    const auto window_size = window->GetBox().GetSize(Rml::BoxArea::Padding);
    const auto anchor_pos = anchor->GetAbsoluteOffset(Rml::BoxArea::Border);
    const auto anchor_size = anchor->GetBox().GetSize(Rml::BoxArea::Border);

    // Height + clamp estimate only; actual width comes from shrink-to-fit against the window.
    constexpr float tip_h = 22.f;
    constexpr float tip_w_est = 7.f;
    constexpr float pad_x = 16.f;
    const auto tip_w = static_cast<float>(text.size()) * tip_w_est + pad_x;

    constexpr float gap = 4.f;
    auto left = anchor_pos.x - window_pos.x;
    auto top = anchor_pos.y - window_pos.y - tip_h - gap;

    const auto max_left = std::max(0.f, window_size.x - tip_w);
    left = std::clamp(left, 0.f, max_left);
    if (top < 0.f)
    {
      top = anchor_pos.y - window_pos.y + anchor_size.y + gap;
    }

    icon_tooltip->SetProperty(Rml::PropertyId::Left, Rml::Property(left, Rml::Unit::DP));
    icon_tooltip->SetProperty(Rml::PropertyId::Top, Rml::Property(top, Rml::Unit::DP));
    icon_tooltip->RemoveProperty("display");
  }

  void RmlUserInterface::hide_icon_tooltip()
  {
    if (!icon_tooltip)
    {
      return;
    }

    icon_tooltip->SetProperty("display", "none");
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

    sync_log_view();

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
    if (!root_document || !plugins_content || !plugins_buttons)
    {
      LOG(ERROR, "Cannot allocate tab before the root document is loaded");
      return 0;
    }

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

    RmlComponent tab_component{};
    tab_component.type = ComponentType::container;
    tab_component.element = container;
    component_map[id] = std::move(tab_component);
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
    if (!document)
    {
      LOG(ERROR, "Cannot allocate component with no owner document");
      return 0;
    }

    RmlComponent component;
    auto append = [&](Rml::ElementPtr created)
    {
      component.element = parent_element->AppendChild(std::move(created));
    };

    if (type == ComponentType::container)
    {
      append(document->CreateElement("div"));
    }
    else if (type == ComponentType::row)
    {
      auto element = document->CreateElement("div");
      element->SetClass("plugin-row", true);
      append(std::move(element));
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
      element->AddEventListener(Rml::EventId::Click, new ButtonClickEventListener(this, id));
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
    else if (type == ComponentType::graph_map)
    {
      auto wrap = document->CreateElement("div");
      wrap->SetClass("graph-map-wrap", true);

      auto map = document->CreateElement("div");
      map->SetId(std::format("{}", id));
      map->SetClass("graph-map", true);
      auto *listener = new GraphMapEventListener(this, id);
      map->AddEventListener(Rml::EventId::Mousedown, listener);
      map->AddEventListener(Rml::EventId::Mousemove, listener);
      map->AddEventListener(Rml::EventId::Mouseup, listener);
      map->AddEventListener(Rml::EventId::Click, listener);
      map->AddEventListener(Rml::EventId::Mousescroll, listener);

      auto hover = document->CreateElement("div");
      hover->SetClass("graph-map-hover", true);
      hover->SetProperty("display", "none");

      auto labels = document->CreateElement("div");
      labels->SetClass("graph-map-labels", true);

      auto menu = document->CreateElement("div");
      menu->SetClass("graph-map-context", true);
      menu->SetId(std::format("ctx_{}", id));
      menu->SetProperty("display", "none");
      menu->SetInnerRML(std::format(
          "<button id=\"ctx_add_here_{0}\">Add vertex here</button>"
          "<button id=\"ctx_add_player_{0}\">Add at player</button>"
          "<button id=\"ctx_delete_{0}\">Delete vertex</button>",
          id));

      auto *wrap_el = parent_element->AppendChild(std::move(wrap));
      component.element = wrap_el->AppendChild(std::move(map));
      component.graph_map = GraphMapUiState{};
      component.graph_map.labels_layer = wrap_el->AppendChild(std::move(labels));
      component.graph_map.hover_label = wrap_el->AppendChild(std::move(hover));
      component.graph_map.context_menu = wrap_el->AppendChild(std::move(menu));

      auto bind_ctx = [&](const char *prefix, uint32_t action)
      {
        if (auto *btn = component.graph_map.context_menu->GetElementById(std::format("{}_{}", prefix, id)))
        {
          btn->AddEventListener(Rml::EventId::Click, new GraphMapContextButtonListener(this, id, action));
        }
      };
      bind_ctx("ctx_add_here", 0);
      bind_ctx("ctx_add_player", 1);
      bind_ctx("ctx_delete", 2);
    }

    if (!component.element)
    {
      return 0;
    }

    component.type = type;
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
      child->SetInnerRML(escape_rml_text(text));
    }
    else if (component->type == ComponentType::button)
    {
      element->SetInnerRML(escape_rml_text(text));
    }
    else if (component->type == ComponentType::label)
    {
      element->SetInnerRML(escape_rml_text(text));
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
      component->dropdown.change_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::register_button_click_handler(uint64_t component_id, std::function<void()> handler)
  {
    if (auto *component = find_component(component_id))
    {
      component->button.click_handlers.push_back(std::move(handler));
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

    auto handlers = component->dropdown.change_handlers;
    for (auto &handler : handlers)
    {
      handler(index);
    }
  }

  void RmlUserInterface::on_button_component_clicked(uint64_t component_id)
  {
    auto *component = find_component(component_id);
    if (!component)
    {
      return;
    }

    auto handlers = component->button.click_handlers;
    for (auto &handler : handlers)
    {
      handler();
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

  RmlComponent *RmlUserInterface::find_graph_map_component(uint64_t component_id)
  {
    auto *component = find_component(component_id);
    if (!component || component->type != ComponentType::graph_map)
    {
      return nullptr;
    }
    return component;
  }

  void RmlUserInterface::rebuild_graph_map_labels(RmlComponent *component)
  {
    if (!component)
    {
      return;
    }

    auto &state = component->graph_map;
    if (!state.labels_layer || !component->element)
    {
      return;
    }

    while (state.labels_layer->HasChildNodes())
    {
      state.labels_layer->RemoveChild(state.labels_layer->GetFirstChild());
    }

    auto size_vec = component->element->GetBox().GetSize(Rml::BoxArea::Padding);
    if (size_vec.x <= 1.f || size_vec.y <= 1.f)
    {
      return;
    }

    constexpr float pad = 6.f;
    auto radius = static_cast<float>(state.radius_tiles ? state.radius_tiles : 1);
    auto inner = std::min(std::max(1.f, size_vec.x - pad * 2.f), std::max(1.f, size_vec.y - pad * 2.f));
    auto scale = inner / (radius * 2.f);
    auto *doc = state.labels_layer->GetOwnerDocument();
    if (!doc)
    {
      return;
    }

    for (auto &node : state.nodes)
    {
      auto px = (static_cast<float>(node.x) - static_cast<float>(state.center_x)) * scale + size_vec.x * 0.5f;
      auto py = (static_cast<float>(state.center_y) - static_cast<float>(node.y)) * scale + size_vec.y * 0.5f;
      auto label = doc->CreateElement("div");
      label->SetClass("graph-map-id", true);
      if (node.id == state.selected_id)
      {
        label->SetClass("graph-map-id-selected", true);
      }
      label->SetInnerRML(std::format("{}", node.id));
      label->SetProperty(Rml::PropertyId::Left, Rml::Property(px + 7.f, Rml::Unit::PX));
      label->SetProperty(Rml::PropertyId::Top, Rml::Property(py - 11.f, Rml::Unit::PX));
      state.labels_layer->AppendChild(std::move(label));
    }
  }

  void RmlUserInterface::update_graph_map(uint64_t component_id, uint32_t center_x, uint32_t center_y, uint32_t radius_tiles,
      uint32_t selected_id, const std::vector<GraphMapNode> &nodes,
      const std::vector<GraphMapEdge> &edges,
      const std::vector<GraphMapObject> &objects)
  {
    auto *component = find_graph_map_component(component_id);
    if (!component)
    {
      return;
    }

    auto &state = component->graph_map;
    state.center_x = center_x;
    state.center_y = center_y;
    state.radius_tiles = radius_tiles ? radius_tiles : 1;
    state.selected_id = selected_id;
    state.nodes = nodes;
    state.edges = edges;
    state.objects = objects;
    if (state.hovered_object >= static_cast<int32_t>(state.objects.size()))
    {
      state.hovered_object = -1;
      if (state.hover_label)
      {
        state.hover_label->SetProperty("display", "none");
      }
    }

    rebuild_graph_map_labels(component);
  }

  void RmlUserInterface::update_graph_map_primitives(uint64_t component_id, const std::vector<GraphMapPrimitive> &primitives)
  {
    auto *component = find_graph_map_component(component_id);
    if (!component)
    {
      return;
    }

    component->graph_map.primitives = primitives;
  }

  void RmlUserInterface::register_graph_map_select_handler(uint64_t component_id, std::function<void(uint32_t)> handler)
  {
    if (auto *component = find_graph_map_component(component_id))
    {
      component->graph_map.select_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::register_graph_map_link_handler(uint64_t component_id, std::function<void(uint32_t, uint32_t)> handler)
  {
    if (auto *component = find_graph_map_component(component_id))
    {
      component->graph_map.link_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::register_graph_map_object_link_handler(uint64_t component_id,
      std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)> handler)
  {
    if (auto *component = find_graph_map_component(component_id))
    {
      component->graph_map.object_link_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::register_graph_map_background_handler(uint64_t component_id, std::function<void(uint32_t, uint32_t)> handler)
  {
    if (auto *component = find_graph_map_component(component_id))
    {
      component->graph_map.background_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::register_graph_map_context_handler(uint64_t component_id,
      std::function<void(uint32_t, uint32_t, uint32_t, int32_t)> handler)
  {
    if (auto *component = find_graph_map_component(component_id))
    {
      component->graph_map.context_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::register_graph_map_zoom_handler(uint64_t component_id, std::function<void(uint32_t)> handler)
  {
    if (auto *component = find_graph_map_component(component_id))
    {
      component->graph_map.zoom_handlers.push_back(std::move(handler));
    }
  }

  void RmlUserInterface::on_graph_map_context_action(uint64_t component_id, uint32_t action)
  {
    auto *component = find_graph_map_component(component_id);
    if (!component)
    {
      return;
    }

    auto &state = component->graph_map;
    if (state.context_menu)
    {
      state.context_menu->SetProperty("display", "none");
    }
    state.context_open = false;

    if (action == 2 && state.context_vertex_id < 0)
    {
      return;
    }

    auto tile_x = state.context_tile_x;
    auto tile_y = state.context_tile_y;
    auto vertex_id = state.context_vertex_id;
    auto handlers = state.context_handlers;
    for (auto &handler : handlers)
    {
      handler(action, tile_x, tile_y, vertex_id);
    }
  }

  void RmlUserInterface::on_graph_map_event(uint64_t component_id, Rml::Event &event)
  {
    auto *component = find_graph_map_component(component_id);
    if (!component || !component->element)
    {
      return;
    }

    auto &state = component->graph_map;
    auto *element = component->element;
    auto size_vec = element->GetBox().GetSize(Rml::BoxArea::Padding);
    if (size_vec.x <= 1.f || size_vec.y <= 1.f)
    {
      return;
    }

    constexpr float pad = 6.f;
    auto radius = static_cast<float>(state.radius_tiles ? state.radius_tiles : 1);
    auto inner = std::min(std::max(1.f, size_vec.x - pad * 2.f), std::max(1.f, size_vec.y - pad * 2.f));
    auto scale = inner / (radius * 2.f);

    auto abs = element->GetAbsoluteOffset(Rml::BoxArea::Border);
    auto mouse_x = static_cast<float>(event.GetParameter<int>("mouse_x", 0)) - abs.x;
    auto mouse_y = static_cast<float>(event.GetParameter<int>("mouse_y", 0)) - abs.y;
    auto button = event.GetParameter<int>("button", 0);

    auto tile_to_px = [&](uint32_t tile_x, uint32_t tile_y) -> Rml::Vector2f
    {
      return {
        (static_cast<float>(tile_x) - static_cast<float>(state.center_x)) * scale + size_vec.x * 0.5f,
        (static_cast<float>(state.center_y) - static_cast<float>(tile_y)) * scale + size_vec.y * 0.5f
      };
    };

    auto hit_node = [&]() -> std::optional<uint32_t>
    {
      constexpr float hit_radius = 8.f;
      std::optional<uint32_t> best;
      float best_dist = hit_radius * hit_radius;
      for (auto &node : state.nodes)
      {
        auto pos = tile_to_px(node.x, node.y);
        auto dx = pos.x - mouse_x;
        auto dy = pos.y - mouse_y;
        auto dist = dx * dx + dy * dy;
        if (dist <= best_dist)
        {
          best_dist = dist;
          best = node.id;
        }
      }
      return best;
    };

    auto hit_object = [&]() -> std::optional<size_t>
    {
      constexpr float hit_radius = 8.f;
      std::optional<size_t> best;
      float best_dist = hit_radius * hit_radius;
      for (size_t i = 0; i < state.objects.size(); i++)
      {
        auto &obj = state.objects[i];
        auto pos = tile_to_px(obj.x, obj.y);
        auto dx = pos.x - mouse_x;
        auto dy = pos.y - mouse_y;
        auto dist = dx * dx + dy * dy;
        if (dist <= best_dist)
        {
          best_dist = dist;
          best = i;
        }
      }
      return best;
    };

    auto local_to_tile = [&](float lx, float ly) -> std::pair<uint32_t, uint32_t>
    {
      auto tile_x = static_cast<int32_t>(std::lround(static_cast<float>(state.center_x) + (lx - size_vec.x * 0.5f) / scale));
      auto tile_y = static_cast<int32_t>(std::lround(static_cast<float>(state.center_y) - (ly - size_vec.y * 0.5f) / scale));
      return { static_cast<uint32_t>(std::max(0, tile_x)), static_cast<uint32_t>(std::max(0, tile_y)) };
    };

    auto hide_context = [&]()
    {
      if (state.context_menu)
      {
        state.context_menu->SetProperty("display", "none");
      }
      state.context_open = false;
    };

    auto id = event.GetId();
    if (id == Rml::EventId::Mousescroll)
    {
      event.StopPropagation();
      auto dy = event.GetParameter<float>("wheel_delta_y", 0.f);
      if (std::fabs(dy) < 0.01f)
      {
        return;
      }

      auto step = dy < 0.f ? -1 : 1;
      if (std::fabs(dy) >= 2.f)
      {
        step *= 2;
      }

      constexpr int min_radius = 4;
      constexpr int max_radius = 40;
      auto next = std::clamp(static_cast<int>(state.radius_tiles) + step, min_radius, max_radius);
      if (static_cast<uint32_t>(next) == state.radius_tiles)
      {
        return;
      }

      state.radius_tiles = static_cast<uint32_t>(next);
      rebuild_graph_map_labels(component);
      auto radius_tiles = state.radius_tiles;
      auto handlers = state.zoom_handlers;
      for (auto &handler : handlers)
      {
        handler(radius_tiles);
      }
      return;
    }
    if (id == Rml::EventId::Mousedown)
    {
      event.StopPropagation();
      if (button == 1)
      {
        hide_context();
        auto [tile_x, tile_y] = local_to_tile(mouse_x, mouse_y);
        state.context_tile_x = tile_x;
        state.context_tile_y = tile_y;
        state.context_vertex_id = -1;
        if (auto node = hit_node())
        {
          state.context_vertex_id = static_cast<int32_t>(*node);
        }

        if (state.context_menu)
        {
          if (auto *del = state.context_menu->GetElementById(std::format("ctx_delete_{}", component_id)))
          {
            del->SetProperty("display", state.context_vertex_id >= 0 ? "block" : "none");
          }
          state.context_menu->SetProperty(Rml::PropertyId::Left, Rml::Property(mouse_x, Rml::Unit::PX));
          state.context_menu->SetProperty(Rml::PropertyId::Top, Rml::Property(mouse_y, Rml::Unit::PX));
          state.context_menu->RemoveProperty("display");
          state.context_open = true;
        }
        return;
      }

      hide_context();
      if (auto node = hit_node())
      {
        state.dragging = true;
        state.drag_moved = false;
        state.drag_from_id = *node;
        for (auto &n : state.nodes)
        {
          if (n.id == *node)
          {
            auto p = tile_to_px(n.x, n.y);
            state.drag_from_px = p.x;
            state.drag_from_py = p.y;
            break;
          }
        }
        state.drag_to_px = mouse_x;
        state.drag_to_py = mouse_y;
      }
      return;
    }

    if (id == Rml::EventId::Mousemove)
    {
      if (state.dragging)
      {
        event.StopPropagation();
        auto dx = mouse_x - state.drag_from_px;
        auto dy = mouse_y - state.drag_from_py;
        if (dx * dx + dy * dy > 16.f)
        {
          state.drag_moved = true;
        }
        state.drag_to_px = mouse_x;
        state.drag_to_py = mouse_y;
        return;
      }

      auto obj = hit_object();
      auto next_hover = obj ? static_cast<int32_t>(*obj) : -1;
      if (next_hover != state.hovered_object)
      {
        state.hovered_object = next_hover;
        if (state.hover_label)
        {
          if (next_hover >= 0)
          {
            auto &o = state.objects[static_cast<size_t>(next_hover)];
            auto pos = tile_to_px(o.x, o.y);
            state.hover_label->SetInnerRML(o.name.empty() ? std::format("{}", o.object_id) : o.name);
            state.hover_label->SetProperty(Rml::PropertyId::Left, Rml::Property(pos.x + 8.f, Rml::Unit::PX));
            state.hover_label->SetProperty(Rml::PropertyId::Top, Rml::Property(pos.y - 14.f, Rml::Unit::PX));
            state.hover_label->RemoveProperty("display");
          }
          else
          {
            state.hover_label->SetProperty("display", "none");
          }
        }
      }
      return;
    }

    if (id == Rml::EventId::Mouseup)
    {
      if (!state.dragging)
      {
        return;
      }

      event.StopPropagation();
      auto from = state.drag_from_id;
      auto moved = state.drag_moved;
      state.dragging = false;
      state.drag_moved = false;
      if (moved)
      {
        state.suppress_click = true;
        if (auto to = hit_node(); to && *to != from)
        {
          auto handlers = state.link_handlers;
          for (auto &handler : handlers)
          {
            handler(from, *to);
          }
        }
        else if (auto obj = hit_object())
        {
          auto o = state.objects[*obj];
          auto handlers = state.object_link_handlers;
          for (auto &handler : handlers)
          {
            handler(from, o.object_id, o.x, o.y, o.option);
          }
        }
      }
      return;
    }

    if (id == Rml::EventId::Click)
    {
      event.StopPropagation();
      if (state.suppress_click)
      {
        state.suppress_click = false;
        return;
      }
      if (state.dragging)
      {
        return;
      }

      hide_context();

      if (auto node = hit_node())
      {
        state.selected_id = *node;
        auto handlers = state.select_handlers;
        for (auto &handler : handlers)
        {
          handler(*node);
        }
        return;
      }

      auto [tile_x, tile_y] = local_to_tile(mouse_x, mouse_y);
      auto handlers = state.background_handlers;
      for (auto &handler : handlers)
      {
        handler(tile_x, tile_y);
      }
    }
  }
} // namespace crs
