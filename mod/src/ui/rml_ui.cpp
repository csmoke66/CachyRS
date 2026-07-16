#include "ui.h"

#include <format>
#include <iostream>

#include "log.h"
#include "util.h"
#include "version.hpp"

#include <RmlUi_Platform_SDL.h>

namespace crs
{
    bool CachySystemInterface::LogMessage(Rml::Log::Type type, const Rml::String &message)
    {
        LOG(UI, message);
        return true;
    }

    class SwitchTabEventHandler : public Rml::EventListener
    {
    private:
        Rml::Element **current_tab = nullptr;
        Rml::Element **current_content = nullptr;
        Rml::Element *tab = nullptr;
        Rml::Element *content = nullptr;

    public:
        SwitchTabEventHandler(
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

    public:
        void ProcessEvent(Rml::Event &event)
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
    };

    class ToggleFeatureEventListener : public Rml::EventListener
    {
    private:
        bool *val;

    public:
        ToggleFeatureEventListener(bool *val)
        {
            this->val = val;
        }

    public:
        void ProcessEvent(Rml::Event &event) override
        {
            auto *checkbox = event.GetCurrentElement();
            *val = checkbox->HasAttribute("checked");
        }
    };

    class ClickDomNodeEventListener : public Rml::EventListener
    {

    private:
        RmlUserInterface* parent;
        std::shared_ptr<DomNode> node;

    public:
        ClickDomNodeEventListener(RmlUserInterface* parent, std::shared_ptr<DomNode> node)
        {
            this->parent = parent;
            this->node = node;
        }

    private:
        
    public:
        void ProcessEvent(Rml::Event &event) override
        {
            event.StopPropagation();

            parent->inspect_dom_node(node);
        }
    };

    class ToggleDomNodeEventListener : public Rml::EventListener
    {
    private:
        Rml::Element *element;

    public:
        ToggleDomNodeEventListener(Rml::Element *element)
        {
            this->element = element;
        }

    public:
        void ProcessEvent(Rml::Event &event) override
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
    };

    RmlUserInterface::RmlUserInterface()
    {
        root_dom_node = std::make_shared<DomNode>("root", "root");
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

        Rml::Debugger::Initialise(context);
        load_fonts();
        reload();
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
        }

        root_document = context->LoadDocument(config_folder + "rmlui/main.html");
        if (root_document)
        {
            auto *title = root_document->GetElementById("title-version");
            title->SetInnerRML(std::format("{} ({})", FEATURE_VERSION, CACHYRS_VERSION));

            home_tab_button = root_document->GetElementById("home_tab_button");
            home_content = root_document->GetElementById("home_content");

            plugins_tab_button = root_document->GetElementById("plugins_tab_button");
            plugins_content = root_document->GetElementById("plugins_content");

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

            root_dom_node->element = debug_content->GetElementById("dom-tree");
            root_document->Show();
        }
    }

    void RmlUserInterface::process(SDL_Event *event)
    {
        wants_input_last = !RmlSDL::InputEventHandler(context, sdl_window, *event);
    }

    bool RmlUserInterface::wants_input()
    {
        return wants_input_last;
    }

    void RmlUserInterface::build_dom_node(std::shared_ptr<DomNode> node, int depth)
    {
        if (!node->is_built)
        {
            auto element = root_document->CreateElement("div");

            std::string inner_rml;
            inner_rml += std::format("<div><span class=\"dom-node\">&lt;</span><span class=\"dom-node-type\">{}</span>", node->type);

            auto &values = node->values;
            if (!values.empty())
            {
                inner_rml += "<span>&nbsp;</span>";
            }

            std::vector<DomValue*> to_render;
            for (auto& value : values)
            {
                if (!value->hidden)
                {
                    to_render.push_back(value.get());
                }
            }

            for (auto i = 0; i < to_render.size(); i++)
            {
                auto is_last = (i == to_render.size() - 1);
                inner_rml += std::format("<span class=\"dom-node-key\">{}</span><span class=\"dom-node\">=</span><span class=\"dom-node-value\">&quot;{}&quot;</span>", values[i]->name, values[i]->to_string());
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
            node->wrapper_element = node->parent->element->AppendChild(std::move(element));
            node->element = anchor;

            node->wrapper_element->AddEventListener(Rml::EventId::Click, new ClickDomNodeEventListener(this, node));
            node->wrapper_element->AddEventListener(Rml::EventId::Dblclick, new ToggleDomNodeEventListener(node->element));

            node->is_built = true;
        }

        for (auto c : node->children)
        {
            c.second->parent = node;
            build_dom_node(c.second, depth + 1);
        }
    }

    void RmlUserInterface::add_dom_node(std::shared_ptr<DomNode> node)
    {
        node->parent = root_dom_node;
        root_dom_node->children[node->id] = node;
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
            element->SetInnerRML(std::format("<span>{}</span>:<span>{}</span>", value->name, value->to_string()));
            dom_inspector_content->AppendChild(std::move(element));
        }
    }

    void RmlUserInterface::render()
    {
        context->Update();

        Backend::BeginFrame();
        context->Render();
        Backend::PresentFrame();
    }
}