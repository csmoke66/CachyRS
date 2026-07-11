#include "cachy_rmlui.h"
#include "cachy.h"
#include "not_cachy.h"
#include <iostream>

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

bool CachySystemInterface::LogMessage(Rml::Log::Type type, const Rml::String &message)
{
    RS.log_stream << message << std::endl;
    RS.flush_logs();
    return true;
}

void CachyRS::load_fonts()
{
    struct FontFace
    {
        const char *filename;
        bool fallback_face;
    };

    const Rml::String directory = resolve_configuration("rmlui/assets/");
    FontFace font_faces[] = {
        {"RobotoMono-Regular.ttf", true},
    };

    for (const FontFace &face : font_faces)
    {
        Rml::LoadFontFace(directory + face.filename, face.fallback_face);
    }
}

void CachyRS::reload_ui()
{
    if (rmlui_root_document)
    {
        rmlui_root_document->ReloadStyleSheet();
        rmlui_root_document->Close();
        rmlui_root_document = nullptr;
        rmlui_debug_projection = nullptr;
        rmlui_selected_tab_button = nullptr;
        rmlui_selected_content = nullptr;
    }

    rmlui_root_document = rmlui_context->LoadDocument(resolve_configuration("rmlui/main.html"));
    if (rmlui_root_document)
    {
        auto* title = rmlui_root_document->GetElementById("title-version");
        title->SetInnerRML(std::format("{} ({})", FEATURE_VERSION, CACHYRS_VERSION));

        rmlui_home_tab_button = rmlui_root_document->GetElementById("home_tab_button");
        rmlui_home_content = rmlui_root_document->GetElementById("home_content");

        rmlui_plugins_tab_button = rmlui_root_document->GetElementById("plugins_tab_button");
        rmlui_plugins_content = rmlui_root_document->GetElementById("plugins_content");

        rmlui_debug_tab_button = rmlui_root_document->GetElementById("debug_tab_button");
        rmlui_debug_content = rmlui_root_document->GetElementById("debug_content");
        rmlui_debug_projection = rmlui_root_document->GetElementById("projection");

        rmlui_home_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                         &rmlui_selected_tab_button, &rmlui_selected_content,
                                                                         rmlui_home_tab_button, rmlui_home_content));

        rmlui_plugins_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                            &rmlui_selected_tab_button, &rmlui_selected_content,
                                                                            rmlui_plugins_tab_button, rmlui_plugins_content));

        rmlui_debug_tab_button->AddEventListener(Rml::EventId::Click, new SwitchTabEventHandler(
                                                                          &rmlui_selected_tab_button, &rmlui_selected_content,
                                                                          rmlui_debug_tab_button, rmlui_debug_content));

        auto rmlui_debug_player_overlay_cb = rmlui_root_document->GetElementById("player_overlay_checkbox");
        rmlui_debug_player_overlay_cb->AddEventListener(Rml::EventId::Change, new ToggleFeatureEventListener(&player_overlay_on));

        rmlui_root_document->Show();
    }
}

void CachyRS::init_rmlui(int window_width, int window_height)
{
    if (!Backend::Initialize(NRS.sdl_window(), window_width, window_height))
    {
        return;
    }

    Rml::SetSystemInterface(&rmlui_system_interface);
    Rml::SetRenderInterface(Backend::GetRenderInterface());

    Rml::Initialise();

    rmlui_context = Rml::CreateContext("main", Rml::Vector2i(window_width, window_height));
    if (!rmlui_context)
    {
        Rml::Shutdown();
        Backend::Shutdown();
        return;
    }

    Rml::Debugger::Initialise(rmlui_context);
    load_fonts();
    reload_ui();
}

void CachyRS::update_rmlui()
{
    if (auto scene = NRS.scene_003())
    {
        auto &matrix = scene->projection_matrix;
        if (auto text = RS.rmlui_debug_projection)
        {
            text->SetInnerRML(std::format(
                "{:8.4f} {:8.4f} {:8.4f} {:8.4f}\n"
                "{:8.4f} {:8.4f} {:8.4f} {:8.4f}\n"
                "{:8.4f} {:8.4f} {:8.4f} {:8.4f}\n"
                "{:8.4f} {:8.4f} {:8.4f} {:8.4f}",
                matrix.flat[0], matrix.flat[1], matrix.flat[2], matrix.flat[3],
                matrix.flat[4], matrix.flat[5], matrix.flat[6], matrix.flat[7],
                matrix.flat[8], matrix.flat[9], matrix.flat[10], matrix.flat[11],
                matrix.flat[12], matrix.flat[13], matrix.flat[14], matrix.flat[15]));
        }
    }
}