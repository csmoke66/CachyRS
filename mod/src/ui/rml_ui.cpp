#include "ui.h"

#include <format>
#include <iostream>

#include "log.h"
#include "util.h"
#include "version.hpp"

#include <RmlUi_Platform_SDL.h>

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
        debug_projection = nullptr;
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
        debug_projection = root_document->GetElementById("projection");
        debug_item_containers = root_document->GetElementById("item_containers");

        home_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                   &selected_tab_button, &selected_content,
                                                                   home_tab_button, home_content));

        plugins_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                      &selected_tab_button, &selected_content,
                                                                      plugins_tab_button, plugins_content));

        debug_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                    &selected_tab_button, &selected_content,
                                                                    debug_tab_button, debug_content));

        auto rmlui_debug_player_overlay_cb = root_document->GetElementById("player_overlay_checkbox");
        rmlui_debug_player_overlay_cb->AddEventListener(Rml::EventId::Change, new ToggleFeatureEventListener(&player_overlay_on));

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

void RmlUserInterface::render()
{
    context->Update();

    Backend::BeginFrame();
    context->Render();
    Backend::PresentFrame();
}

void RmlUserInterface::propagate(const UserInterfaceState &state)
{
    auto &matrix = state.projection_matrix;
    if (auto text = debug_projection)
    {
        text->SetInnerRML(std::format(
            "<tr>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "</tr>"
            "<tr>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "</tr>"
            "<tr>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "</tr>"
            "<tr>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "<td>{:8.4f}</td>"
            "</tr>",
            matrix.flat[0], matrix.flat[1], matrix.flat[2], matrix.flat[3],
            matrix.flat[4], matrix.flat[5], matrix.flat[6], matrix.flat[7],
            matrix.flat[8], matrix.flat[9], matrix.flat[10], matrix.flat[11],
            matrix.flat[12], matrix.flat[13], matrix.flat[14], matrix.flat[15]));
    }

    if (auto containers = debug_item_containers)
    {
        while (containers->HasChildNodes())
        {
            containers->RemoveChild(containers->GetFirstChild());
        }

        for (auto &container : state.item_containers)
        {
            if (!container.items.empty())
            {
                auto container_div = root_document->CreateElement("div");

                auto container_id_div = root_document->CreateElement("h3");
                container_id_div->SetInnerRML(std::format("{}", container.id));
                container_div->AppendChild(std::move(container_id_div));

                auto item_container_div = root_document->CreateElement("div");

                auto item_container_hr = root_document->CreateElement("span");
                item_container_hr->SetClass("hr-custom", true);
                item_container_div->AppendChild(std::move(item_container_hr));

                for (auto &item : container.items)
                {
                    auto item_div = root_document->CreateElement("div");
                    auto item_span = root_document->CreateElement("span");
                    item_span->SetInnerRML(std::format("{}:{}", item.id, item.amount));

                    item_div->AppendChild(std::move(item_span));
                    item_container_div->AppendChild(std::move(item_div));
                }

                container_div->AppendChild(std::move(item_container_div));
                containers->AppendChild(std::move(container_div));

                auto container_br = root_document->CreateElement("br");
                containers->AppendChild(std::move(container_br));
            }
        }
    }
}

bool RmlUserInterface::get_bool(UserVariable var)
{
    if (var == UserVariable::player_overlay)
    {
        return player_overlay_on;
    }

    return false;
}

void RmlUserInterface::set_bool(UserVariable var, bool b)
{
    if (var == UserVariable::player_overlay)
    {
        player_overlay_on = b;
    }
}