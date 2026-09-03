#include "ui.h"

#include <atomic>
#include <map>
#include <vector>

#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include <RmlUi_Backend.h>

namespace crs
{
  class RmlUserInterface;

  class CachySystemInterface : public Rml::SystemInterface
  {
  public:
    bool LogMessage(Rml::Log::Type type, const Rml::String &message);
  };

  struct RmlDomValue
  {
    Rml::Element *element = nullptr;
  };

  struct RmlDomNode
  {
    Rml::Element *wrapper_element = nullptr;
    Rml::Element *element = nullptr;
    std::map<std::string, RmlDomValue> dom_values;
  };

  //
  // An event listener for swapping visible content based on a selection.
  //
  class OwnedEventListener : public Rml::EventListener
  {
  private:
    size_t attachment_count = 0;

  public:
    void OnAttach(Rml::Element *) override
    {
      ++attachment_count;
    }

    void OnDetach(Rml::Element *) override
    {
      if (--attachment_count == 0)
      {
        delete this;
      }
    }
  };

  class SwitchTabEventHandler : public OwnedEventListener
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
        Rml::Element *content);

  public:
    void ProcessEvent(Rml::Event &event);
  };

  //
  // An event listener for clicking the refresh UI button. Reloads
  // the UI entirely.
  //
  class RefreshEventHandler : public OwnedEventListener
  {
  private:
    RmlUserInterface *rml_ui;

  public:
    RefreshEventHandler(RmlUserInterface *rml_ui);

  public:
    void ProcessEvent(Rml::Event &event) override;
  };

  //
  // An event listener that keeps track of a checkbox's state.
  //
  class ToggleFeatureEventListener : public OwnedEventListener
  {
  private:
    bool *val;

  public:
    ToggleFeatureEventListener(bool *val);

  public:
    void ProcessEvent(Rml::Event &event) override;
  };

  //
  // A generic handler for clicking on a DOM node. Glues together stuff
  // like selected entity overlay.
  //
  class DomNodeEventListener : public OwnedEventListener
  {
  private:
    RmlUserInterface *parent;
    std::shared_ptr<DomNode> node;

  public:
    DomNodeEventListener(RmlUserInterface *parent, std::shared_ptr<DomNode> node);

  private:
  public:
    void ProcessEvent(Rml::Event &event) override;
  };

  //
  // Allows for DOM nodes to be toggled visible/invisible.
  //
  class ToggleDomNodeEventListener : public OwnedEventListener
  {
  private:
    Rml::Element *element;

  public:
    ToggleDomNodeEventListener(Rml::Element *element);

  public:
    void ProcessEvent(Rml::Event &event) override;
  };

  //
  // Allows an element to be dragged on the screen using the mouse.
  //
  class DragWindowEventListener : public OwnedEventListener
  {
  private:
    Rml::Element *element;
    Rml::Element *window;
    int drag_offset_x = 0;
    int drag_offset_y = 0;

  public:
    DragWindowEventListener(Rml::Element *element, Rml::Element *window);

  public:
    void ProcessEvent(Rml::Event &event) override;
  };

  class DropDownChangedEventListener : public OwnedEventListener
  {
  private:
    RmlUserInterface *parent;
    uint64_t component_id;

  public:
    DropDownChangedEventListener(RmlUserInterface *parent, uint64_t component_id);

  public:
    void ProcessEvent(Rml::Event &event) override;
  };

  //
  // Our decorator which handles visibility exposure using the 'render_frame' attribute.
  //
  // See: RmlUserInterface::get_render_frame()
  //
  class VisibilityTrackerDecorator : public Rml::Decorator
  {
  public:
    RmlUserInterface *parent;

  public:
    Rml::DecoratorDataHandle GenerateElementData(Rml::Element *element, Rml::BoxArea element_data) const override;
    void ReleaseElementData(Rml::DecoratorDataHandle element_data) const override;
    bool IsElementOnScreen(Rml::Element *element, Rml::Context *context) const;
    void RenderElement(Rml::Element *element, Rml::DecoratorDataHandle element_data) const override;
  };

  //
  // Our instancer for creating visiblity tracker decorations.
  //
  class VisibilityTrackerInstancer : public Rml::DecoratorInstancer
  {
  public:
    RmlUserInterface *parent;

  public:
    Rml::SharedPtr<Rml::Decorator> InstanceDecorator(
        const Rml::String &name,
        const Rml::PropertyDictionary &properties,
        const Rml::DecoratorInstancerInterface &instancer_interface) override;
  };

  struct RmlComponent
  {
  public:
    ComponentType type;
    Rml::Element *element;

  public:
    struct
    {
      std::vector<std::function<void(int)>> change_handlers;
    } dropdown;
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
    std::unique_ptr<VisibilityTrackerInstancer> visibility_tracker_instancer;

    Rml::Element *selected_tab_button = nullptr;
    Rml::Element *selected_content = nullptr;

    Rml::Element *selected_plugin_tab_button = nullptr;
    Rml::Element *selected_plugin_content = nullptr;

    Rml::Element *home_tab_button = nullptr;
    Rml::Element *home_content = nullptr;

    Rml::Element *plugins_tab_button = nullptr;
    Rml::Element *plugins_content = nullptr;
    Rml::Element *plugins_buttons = nullptr;

    Rml::Element *debug_tab_button = nullptr;
    Rml::Element *debug_content = nullptr;

    Rml::Element *dom_inspector_content = nullptr;

    Rml::Element *last_hovered = nullptr;

  public:
    std::unique_ptr<DomTreeListener> dom_tree_listener;
    std::shared_ptr<DomNode> root_dom_node;
    std::map<std::shared_ptr<DomNode>, RmlDomNode> dom_nodes;

  private:
    std::atomic<uint64_t> component_allocation = 1;
    std::map<uint64_t, Rml::ElementDocument *> document_map;
    std::map<uint64_t, RmlComponent> component_map;

  private:
    std::vector<std::function<void()>> reload_callbacks;

  private:
    uint64_t render_frame = 0;

  public:
    RmlUserInterface();
    ~RmlUserInterface();

  private:
    bool player_overlay_on = false;

  private:
    void load_fonts();
    Rml::ElementDocument *load_document(const std::string &path);

  public:
    void pre_init();
    void init(const std::string &version, const std::string &config_folder, SDL_Window *window, int width, int height) override;
    void reload() override;
    void add_reload_callback(std::function<void()> function) override;

  public:
    void process(SDL_Event *event) override;
    bool wants_input() override;

  public:
    void set_listener(std::unique_ptr<DomTreeListener> listener) override;
    RmlDomNode *get_rml_dom_node(std::shared_ptr<DomNode> node);
    bool build_dom_node(std::shared_ptr<DomNode> node, int depth = 0) override;
    void add_dom_node(std::shared_ptr<DomNode> node) override;
    void remove_dom_node(std::shared_ptr<DomNode> node) override;
    Rml::Element *get_dom_parent(Rml::Element *element);
    void inspect_dom_node(std::shared_ptr<DomNode> node);

  public:
    bool is_rendered(Rml::Element *element);
    void render() override;
    uint64_t get_render_frame();

  public:
    uint64_t allocate_tab(const std::string &name) override;
    uint64_t allocate_component(ComponentType type, uint64_t parent_id) override;
    void update_component_text(uint64_t component_id, std::string text) override;
    void update_component_items(uint64_t component_id, const std::vector<std::string> &items) override;
    bool is_component_checked(uint64_t component_id) override;
    void register_dropdown_change_handler(uint64_t component_id, std::function<void(int32_t)> handler) override;
    void on_dropdown_component_changed(uint64_t component_id, int32_t idx);
    void set_component_visible(uint64_t component_id, bool visible) override;
  };
} // namespace crs
