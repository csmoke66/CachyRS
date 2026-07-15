#pragma once
#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

#include "math.h"
#include "dom.h"

namespace crs
{
    class UserInterface
    {
    public:
        virtual ~UserInterface();

    public:
        virtual void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) = 0;
        virtual void reload();

    public:
        virtual void process(SDL_Event *event) = 0;
        virtual bool wants_input() = 0;

    public:
        virtual void render() = 0;
    };

    class CachySystemInterface : public Rml::SystemInterface
    {
    public:
        bool LogMessage(Rml::Log::Type type, const Rml::String &message);
    };

    class RmlUserInterface : public UserInterface
    {
    private:
        std::string version;
        std::string config_folder;
        SDL_Window *sdl_window;

    private:
        bool wants_input_last = false;

    private:
        CachySystemInterface system_interface;
        Rml::Context *context = nullptr;
        Rml::ElementDocument *root_document = nullptr;

        Rml::Element *selected_tab_button = nullptr;
        Rml::Element *selected_content = nullptr;

        Rml::Element *home_tab_button = nullptr;
        Rml::Element *home_content = nullptr;

        Rml::Element *plugins_tab_button = nullptr;
        Rml::Element *plugins_content = nullptr;

        Rml::Element *debug_tab_button = nullptr;
        Rml::Element *debug_content = nullptr;

    public:
        std::shared_ptr<DomNode> root_dom_node;

    public:
        RmlUserInterface();

    private:
        bool player_overlay_on = false;

    private:
        void load_fonts();

    public:
        void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) override;
        void reload() override;

    public:
        void process(SDL_Event *event) override;
        bool wants_input() override;

    public:
        void build_dom_node(std::shared_ptr<DomNode> node, int depth = 0);

    public:
        void add_dom_node(std::shared_ptr<DomNode> node);

    public:
        void render() override;
    };
}