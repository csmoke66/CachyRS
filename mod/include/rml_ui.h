#include "ui.h"

#include <map>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

namespace crs
{
    struct RmlDomNode
    {
        Rml::Element *wrapper_element = nullptr;
        Rml::Element *element = nullptr;
    };

    class CachySystemInterface : public Rml::SystemInterface
    {
    public:
        bool LogMessage(Rml::Log::Type type, const Rml::String &message);
    };

    class RmlUserInterface : public UserInterface,
                             public DomTree,
                             public std::enable_shared_from_this<RmlUserInterface>
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

        Rml::Element *dom_inspector_content = nullptr;

    public:
        std::unique_ptr<DomTreeListener> dom_tree_listener;
        std::shared_ptr<DomNode> root_dom_node;
        std::map<std::shared_ptr<DomNode>, RmlDomNode> dom_nodes;

    public:
        RmlUserInterface();

    private:
        bool player_overlay_on = false;

    private:
        void load_fonts();

    public:
        void pre_init();
        void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) override;
        void reload() override;

    public:
        void process(SDL_Event *event) override;
        bool wants_input() override;

    public:
        void set_listener(std::unique_ptr<DomTreeListener> listener) override;
        RmlDomNode *get_rmlui_dom_node(std::shared_ptr<DomNode> node);
        void build_dom_node(std::shared_ptr<DomNode> node, int depth = 0) override;
        void add_dom_node(std::shared_ptr<DomNode> node) override;
        void remove_dom_node(std::shared_ptr<DomNode> node) override;

    public:
        void inspect_dom_node(std::shared_ptr<DomNode> node);

    public:
        void render() override;
    };
}