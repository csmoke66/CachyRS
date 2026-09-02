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
    bool CachySystemInterface::LogMessage(Rml::Log::Type type, const Rml::String &message)
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
            {"RobotoMono-Regular.ttf", true},
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

    void RmlUserInterface::init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height)
    {
        this->version = version;
        this->config_folder = config_folder;
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
        if (!!document)
        {
            auto title = document->GetElementById("titlebar");

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

            auto dom_node_ext = get_rml_dom_node(root_dom_node);
            dom_node_ext->element = debug_content->GetElementById("dom-tree");

            auto refresh_button = root_document->GetElementById("refresh_button");
            refresh_button->AddEventListener(Rml::EventId::Click, new RefreshEventHandler(this));

            root_document->Show();
        }

        for (auto &f : this->reload_callbacks)
        {
            f();
        }
    }

    void RmlUserInterface::add_reload_callback(std::function<void()> function)
    {
        this->reload_callbacks.push_back(function);
    }

    void RmlUserInterface::process(SDL_Event *event)
    {
        wants_input_last = !Backend::ProcessEvents(context, event);
    }

    bool RmlUserInterface::wants_input()
    {
        return wants_input_last;
    }

    RmlDomNode *RmlUserInterface::get_rml_dom_node(std::shared_ptr<DomNode> node)
    {
        auto f = dom_nodes.find(node);
        if (f != dom_nodes.end())
        {
            return &f->second;
        }

        dom_nodes[node] = RmlDomNode();
        return &dom_nodes[node];
    }

    void RmlUserInterface::set_listener(std::unique_ptr<DomTreeListener> listener)
    {
        this->dom_tree_listener = std::move(listener);
    }

    bool RmlUserInterface::build_dom_node(std::shared_ptr<DomNode> node, int depth)
    {
        auto dom_node_ext = get_rml_dom_node(node);
        auto rendered = !dom_node_ext->wrapper_element || is_rendered(dom_node_ext->wrapper_element);
        node->visible = rendered;

        if (rendered)
        {
            if (!node->is_built)
            {
                auto element = root_document->CreateElement("div");

                // This creates an instance of VisibilityTrackerDecorator, which allows us
                // to efficiently handle exposing if the div is being rendered on the screen.
                element->SetProperty("decorator", "screen-tracker");

                std::string inner_rml;
                inner_rml += std::format("<div><span class=\"dom-node\">&lt;</span><span class=\"dom-node-type\">{}</span>", node->type);

                auto &values = node->values;
                std::vector<DomValue *> to_render;
                for (auto &value : values)
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

                for (auto i = 0; i < to_render.size(); i++)
                {
                    auto is_last = (i == to_render.size() - 1);
                    inner_rml += std::format("<span class=\"dom-node-key\">{}</span><span class=\"dom-node\">=</span><span class=\"dom-node-value\" id=\"{}\">&quot;{}&quot;</span>", to_render[i]->name, to_render[i]->id, to_render[i]->to_string());
                    if (!is_last)
                    {
                        inner_rml += "<span>&nbsp;</span>";
                    }
                }
                inner_rml += std::format("<span class=\"dom-node\" id=\"{}\">&gt;</span></div>", node->id);
                inner_rml += std::format("<div><span class=\"dom-node\">&lt;/</span><span class=\"dom-node-type\">{}</span><span class=\"dom-node\">&gt;</span></div>", node->type);

                element->SetInnerRML(inner_rml);
                element->SetClass("dom-row", true);

                auto anchor = element->GetElementById(node->id);
                auto parent_node_ext = get_rml_dom_node(node->parent);

                // We map these in our DOM node extension to avoid costly lookups whenever the node
                // is dirty and requires value updating.
                for (auto &value : node->values)
                {
                    dom_node_ext->dom_values[value->id] = {element->GetElementById(value->id)};
                }

                dom_node_ext->wrapper_element = parent_node_ext->element->AppendChild(std::move(element));
                dom_node_ext->element = anchor;

                auto dom_node_event_listener = new DomNodeEventListener(this, node);

                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Click, dom_node_event_listener);
                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Mouseover, dom_node_event_listener);
                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Mouseout, dom_node_event_listener);
                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Keydown, dom_node_event_listener);
                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Keyup, dom_node_event_listener);
                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Dblclick, dom_node_event_listener);
                dom_node_ext->wrapper_element->AddEventListener(Rml::EventId::Dblclick, new ToggleDomNodeEventListener(dom_node_ext->element));

                node->is_built = true;
            }
            else if (node->dirty)
            {
                auto element = dom_node_ext->wrapper_element;

                auto display = element->GetProperty(Rml::PropertyId::Display)->Get<Rml::String>();
                auto visibility = element->GetProperty(Rml::PropertyId::Visibility)->Get<Rml::String>();

                auto context = element->GetContext();
                auto dimensions = context ? context->GetDimensions() : Rml::Vector2i{};

                // If a node is dirty, the visible values need to be updated.
                for (auto &value : node->values)
                {
                    if (!value->hidden && value->dirty)
                    {
                        auto dom_value = dom_node_ext->dom_values.find(value->id);
                        if (dom_value != dom_node_ext->dom_values.end())
                        {
                            dom_value->second.element->SetInnerRML(value->to_string());
                        }

                        value->dirty = false;
                    }
                }

                node->dirty = false;
            }

            for (auto c : node->children)
            {
                c.second->parent = node;

                // We assume all children are in order. If one child is not visible,
                // the ones under it are not either.
                if (!build_dom_node(c.second, depth + 1))
                {
                    break;
                }
            }
        }

        return rendered;
    }

    void RmlUserInterface::add_dom_node(std::shared_ptr<DomNode> node)
    {
        node->parent = root_dom_node;
        root_dom_node->children[node->id] = node;
    }

    void RmlUserInterface::remove_dom_node(std::shared_ptr<DomNode> node)
    {
        auto dom_node_ext = get_rml_dom_node(node);
        if (dom_node_ext->wrapper_element)
        {
            auto parent = dom_node_ext->wrapper_element->GetParentNode();
            parent->RemoveChild(dom_node_ext->wrapper_element);
            dom_nodes.erase(node);
        }
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
                rendered = (frame == this->render_frame);
            }
        }
        return rendered;
    }

    void RmlUserInterface::render()
    {
        this->render_frame += 1;

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
        return this->render_frame;
    }

    uint64_t RmlUserInterface::allocate_tab(const std::string &name)
    {
        auto id = component_allocation++;

        auto container_u = root_document->CreateElement("div");
        container_u->SetClass("sidebar-content", true);
        container_u->SetProperty("display", "none");
        auto container = plugins_content->AppendChild(std::move(container_u));

        auto button_u = root_document->CreateElement("button");
        button_u->SetInnerRML(name);
        button_u->SetClass("sidebar-tab-button", true);
        auto button = plugins_buttons->AppendChild(std::move(button_u));

        button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                          &selected_plugin_tab_button, &selected_plugin_content,
                                                          button, container));

        component_map[id] = RmlComponent{ComponentType::container, container};
        return id;
    }

    uint64_t RmlUserInterface::allocate_component(ComponentType type, uint64_t parent_id)
    {
        auto id = component_allocation++;
        auto parent = component_map.find(parent_id);
        auto has_parent = parent != component_map.end();
        auto parent_element = (has_parent ? parent->second.element : nullptr);
        if (!parent_element)
        {
            LOG(ERROR, "Cannot allocate component with no parent");
            return 0;
        }

        RmlComponent component;
        if (type == ComponentType::container)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("div");
            component.element = parent_element->AppendChild(std::move(u_element));
        }
        else if (type == ComponentType::label)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("div");
            u_element->SetInnerRML(std::format("<span id=\"text_{}\">N/A</span>", id));
            component.element = parent_element->AppendChild(std::move(u_element));
        }
        else if (type == ComponentType::hr)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("div");
            u_element->SetClass("hr-custom", true);
            component.element = parent_element->AppendChild(std::move(u_element));
        }
        else if (type == ComponentType::line)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("div");
            u_element->SetClass("hr-empty", true);
            component.element = parent_element->AppendChild(std::move(u_element));
        }
        else if (type == ComponentType::button)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("button");
            u_element->SetId(std::format("{}", id));
            u_element->SetInnerRML("N/A");
            component.element = parent_element->AppendChild(std::move(u_element));
        }
        else if (type == ComponentType::checkbox)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("div");
            u_element->SetInnerRML(std::format("<label class=\"checkbox-row\"><input type=\"checkbox\" id=\"input_{}\"/><span id=\"text_{}\">N/A</span></label>", id, id));
            component.element = parent_element->AppendChild(std::move(u_element));
        }
        else if (type == ComponentType::dropdown)
        {
            auto document = parent_element->GetOwnerDocument();

            auto u_element = document->CreateElement("select");
            u_element->SetId(std::format("{}", id));
            u_element->AddEventListener(Rml::EventId::Change, new DropDownChangedEventListener(this, id));

            component.element = parent_element->AppendChild(std::move(u_element));
            component.dropdown.change_handlers = std::vector<std::function<void(int)>>();
        }

        if (!!component.element)
        {
            component_map[id] = component;
            return id;
        }
        else
        {
            return 0;
        }
    }

    void RmlUserInterface::update_component_text(uint64_t component_id, std::string text)
    {
        auto kv = component_map.find(component_id);
        if (kv != component_map.end())
        {
            auto element = kv->second.element;
            LOG(INFO, "Update text " << component_id << " " << text << " " << element->GetInnerRML());

            auto child = element->GetElementById(std::format("text_{}", component_id));
            if (!child)
            {
                child = element->GetElementById(std::format("title_{}", component_id));
            }

            if (!!child)
            {
                child->SetInnerRML(text);
            }
        }
    }

    void RmlUserInterface::update_component_items(uint64_t component_id, const std::vector<std::string> &items)
    {
        auto kv = component_map.find(component_id);
        if (kv != component_map.end())
        {
            auto element = kv->second.element;
            auto select_element = dynamic_cast<Rml::ElementFormControlSelect *>(element);
            if (!select_element)
            {
                LOG(ERROR, "Invalid element: " << element);
                return;
            }

            LOG(DEBUG, "Removing children");
            select_element->RemoveAll();

            LOG(DEBUG, "Adding options");
            auto idx = 0;
            for (auto &item : items)
            {
                LOG(DEBUG, item);
                select_element->Add(item, std::format("{}", idx++));
            }
        }
    }

    bool RmlUserInterface::is_component_checked(uint64_t component_id)
    {
        auto kv = component_map.find(component_id);
        if (kv != component_map.end())
        {
            auto element = kv->second.element;
            auto child = element->GetElementById(std::format("input_{}", component_id));
            if (!!child)
            {
                return child->HasAttribute("checked");
            }
        }

        return false;
    }

    void RmlUserInterface::register_dropdown_change_handler(uint64_t component_id, std::function<void(int32_t)> handler)
    {
        auto kv = component_map.find(component_id);
        if (kv != component_map.end())
        {
            kv->second.dropdown.change_handlers.push_back(handler);
        }
    }

    void RmlUserInterface::on_dropdown_component_changed(uint64_t component_id, int32_t idx)
    {
        auto kv = component_map.find(component_id);
        if (kv != component_map.end())
        {
            for (auto &f : kv->second.dropdown.change_handlers)
            {
                f(idx);
            }
        }
    }

    void RmlUserInterface::set_component_visible(uint64_t component_id, bool visible)
    {
        auto kv = component_map.find(component_id);
        if (kv != component_map.end())
        {
            if (visible)
            {
                kv->second.element->RemoveProperty("display");
            }
            else
            {
                kv->second.element->SetProperty("display", "none");
            }
        }
    }
}
