#pragma once
#include "math.h"

#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <game_events.h>
#include <plugin.h>
#include <reversed/reversed.h>

namespace crs
{
  namespace Containers
  {
    inline constexpr uint32_t inventory = 93;
    inline constexpr uint32_t bank = 95;
  } // namespace Containers

  namespace InventoryWidget
  {
    inline constexpr uint16_t parent = 1473;
    inline constexpr uint16_t child = 5;
  } // namespace InventoryWidget

  namespace InventoryOption
  {
    inline constexpr uint32_t drop = 8;
  }

  struct Skill
  {
    uint32_t index = 0;
    std::string_view name;
    uint32_t current = 0;
    uint32_t max = 0;
    uint32_t xp = 0;
  };

  struct StatusBarView
  {
    uint32_t id = 0;
    uint8_t value = 0;
    int32_t display_time = 0;
  };

  struct FriendView
  {
    std::string name;
    std::string previous_name;
    std::string world;
  };

  class ApiPlayer;
  class ApiNpc;
  class ApiContainer;

  class ApiEntity
  {
  private:
    Entity *entity = nullptr;

  public:
    ApiEntity(Entity *entity = nullptr);
    ApiEntity(const ApiEntity &o) = default;
    ApiEntity &operator=(const ApiEntity &o) = default;
    virtual ~ApiEntity() = default;

  public:
    bool valid() const;
    explicit operator bool() const;
    Entity *raw() const;
    EntityType type() const;
    WorldNode *node() const;
    virtual Vec3<float> scene_position() const;
    Vec2<uint32_t> tile_position() const;
    bool rendered() const;
    void set_rendered(bool visible);
    std::optional<ApiPlayer> as_player() const;
    std::optional<ApiNpc> as_npc() const;
  };

  class ApiNamedEntity : public ApiEntity
  {
  private:
    NamedEntity *named = nullptr;

  public:
    ApiNamedEntity(NamedEntity *named = nullptr);
    ApiNamedEntity(const ApiNamedEntity &o) = default;
    ApiNamedEntity &operator=(const ApiNamedEntity &o) = default;

  public:
    NamedEntity *raw_named() const;
    int32_t server_index() const;
    std::string name() const;
    Vec3<float> scene_position() const override;
    bool animation_playing() const;
    int32_t animation_id() const;
    bool moving() const;
    std::vector<StatusBarView> status_bars() const;
    std::optional<uint8_t> status(uint32_t bar_id) const;
  };

  class ApiPlayer : public ApiNamedEntity
  {
  private:
    Player *player = nullptr;

  public:
    ApiPlayer(Player *player = nullptr);
    ApiPlayer(const ApiPlayer &o) = default;
    ApiPlayer &operator=(const ApiPlayer &o) = default;

  public:
    static ApiPlayer invalid();
    Player *raw_player() const;
    int32_t combat_level() const;
    int32_t skill_level() const;
    bool is_self() const;
    bool is_friend() const;
  };

  class ApiNpc : public ApiNamedEntity
  {
  private:
    Npc *npc = nullptr;

  public:
    ApiNpc(Npc *npc = nullptr);
    ApiNpc(const ApiNpc &o) = default;
    ApiNpc &operator=(const ApiNpc &o) = default;

  public:
    Npc *raw_npc() const;
    int32_t cache_id() const;
    uint32_t visible_level() const;
    void interact(uint32_t option = 0) const;
  };

  template <typename T>
  class ApiEventList
  {
  private:
    uint64_t token = 0;
    std::map<uint64_t, T> functions;

  public:
    FINLINE uint64_t reg(T function)
    {
      auto id = token++;
      functions[id] = std::move(function);
      return id;
    }

  public:
    FINLINE void iterate(std::function<void(T &)> callback)
    {
      for (auto &[id, fn] : functions)
      {
        callback(fn);
      }
    }
  };

  class ApiComponent
  {
  protected:
    PluginApi api{};
    uint64_t id = static_cast<uint64_t>(-1);

  public:
    ApiComponent(PluginApi api, uint64_t id);
    ApiComponent(const ApiComponent &o) = default;
    ApiComponent &operator=(const ApiComponent &o) = default;

  public:
    uint64_t get_id() const;

  public:
    void set_visible(bool visible);
  };

  class ApiLabel : public ApiComponent
  {
  public:
    ApiLabel();
    ApiLabel(PluginApi api, uint64_t id);
    ApiLabel(const ApiLabel &o) = default;
    ApiLabel &operator=(const ApiLabel &o) = default;

  public:
    void set_text(const std::string &text);
  };

  class ApiHr : public ApiComponent
  {
  public:
    ApiHr(PluginApi api, uint64_t id);
    ApiHr(const ApiHr &o) = default;
    ApiHr &operator=(const ApiHr &o) = default;
  };

  class ApiCheckBox : public ApiComponent
  {
  public:
    ApiCheckBox();
    ApiCheckBox(PluginApi api, uint64_t id);
    ApiCheckBox(const ApiCheckBox &o) = default;
    ApiCheckBox &operator=(const ApiCheckBox &o) = default;

  public:
    bool is_checked() const;
    void set_checked(bool checked);
  };

  class ApiDropDown : public ApiComponent
  {
  private:
    int32_t selected = 0;
    std::vector<std::function<void(int32_t)>> change_handlers;

  public:
    ApiDropDown();
    ApiDropDown(PluginApi api, uint64_t id);
    ApiDropDown(const ApiDropDown &o) = default;
    ApiDropDown &operator=(const ApiDropDown &o) = default;

  public:
    void fire_changed(int32_t idx);
    void on_changed(std::function<void(int32_t)> handler);
    int32_t get_selected() const;
    bool is_selected(int32_t index) const;
    void set_selected(int32_t index);
  };

  struct DropDownContentOption
  {
    std::string name;
    ApiContainer *container = nullptr;
  };

  class DropDownContentChanger
  {
  private:
    std::shared_ptr<ApiDropDown> dropdown;
    ApiContainer *last_visible = nullptr;
    std::vector<ApiContainer *> containers;

  private:
    void on_changed(int32_t idx);

  public:
    int32_t get_selected() const;
    bool is_selected(int32_t idx) const;
    void reset();

  public:
    static std::unique_ptr<DropDownContentChanger> new_changer(
        uint64_t parent,
        std::vector<DropDownContentOption> options);
  };

  class ApiContainer : public ApiComponent
  {
  public:
    ApiContainer();
    ApiContainer(PluginApi api, uint64_t id);
    ApiContainer(const ApiContainer &o) = default;
    ApiContainer &operator=(const ApiContainer &o) = default;

  public:
    ApiContainer add_container();
    ApiLabel add_label(const std::string &text);
    ApiHr add_hr();
    ApiCheckBox add_checkbox(const std::string &text);
    std::shared_ptr<ApiDropDown> add_dropdown(std::vector<std::string> options);
  };

  class ApiItem
  {
  private:
    uint16_t parent_widget = 0xffff;
    uint16_t child_widget = 0xffff;
    int32_t slot = -1;
    int32_t id = -1;
    int32_t amount = 0;

  public:
    ApiItem() = default;
    ApiItem(uint16_t parent_widget, uint16_t child_widget, int32_t slot, int32_t id, int32_t amount);
    ApiItem(const ApiItem &o) = default;
    ApiItem &operator=(const ApiItem &o) = default;

  public:
    MenuActionArgs create_menu_action_args(int index) const;
    void interact(uint32_t option = 0, uint32_t handler = 0) const;
    void override_interact(uint32_t option = 0, uint32_t handler = 0) const;
    void select() const;

  public:
    bool empty() const;
    int32_t get_id() const;
    int32_t get_amount() const;
    int32_t get_slot() const;
  };

  class ApiItemContainer
  {
  private:
    uint32_t container_id = 0;
    uint32_t capacity = 0;
    std::vector<ApiItem> items;

  public:
    ApiItemContainer();
    ApiItemContainer(uint32_t container_id, uint32_t capacity, const std::vector<ApiItem> &items);
    ApiItemContainer(uint32_t capacity, const std::vector<ApiItem> &items);
    ApiItemContainer(const ApiItemContainer &o) = default;
    ApiItemContainer &operator=(const ApiItemContainer &o) = default;

  public:
    uint32_t id() const;
    const std::vector<ApiItem> &all() const;
    std::optional<ApiItem> first(std::function<bool(const ApiItem &)> conditional) const;
    std::optional<ApiItem> find(int32_t item_id) const;
    size_t count(std::function<bool(const ApiItem &)> conditional = [](const ApiItem &)
    {
      return true;
    }) const;
    bool contains(std::function<bool(const ApiItem &)> conditional) const;
    bool contains(int32_t item_id) const;
    bool is_full() const;
    std::vector<ApiItem> filter(std::function<bool(const ApiItem &)> condition = [](const ApiItem &)
    {
      return true;
    }) const;
  };

  class ApiWidget
  {
  private:
    Widget *widget = nullptr;

  public:
    ApiWidget(Widget *widget = nullptr);
    ApiWidget(const ApiWidget &o) = default;
    ApiWidget &operator=(const ApiWidget &o) = default;

  public:
    bool valid() const;
    explicit operator bool() const;
    Widget *raw() const;
    uint16_t parent_id() const;
    uint16_t child_id() const;
    WidgetType type() const;
    uint32_t x() const;
    uint32_t y() const;
    uint32_t width() const;
    uint32_t height() const;
    std::optional<ApiWidget> parent() const;
    std::vector<ApiWidget> children() const;
  };

  class Boot
  {
  public:
    static std::string name();
    static void init();
    static void init_ui();
  };

  class Api
  {
  public:
    static void init(crs::InitType type, Plugin *loaded, std::function<void()> first_initializer, std::function<void()> initializer);
    static void force_on();
    static bool enabled();

  public: // UI
    static uint64_t root_plugin_component_id();

    static ApiContainer add_container(uint64_t parent_id);
    static ApiContainer add_container();

    static ApiLabel add_label(uint64_t parent_id, const std::string &text);
    static ApiLabel add_label(const std::string &text);

    static ApiHr add_hr(uint64_t parent_id);
    static ApiHr add_hr();

    static ApiCheckBox add_checkbox(uint64_t parent_id, const std::string &text);
    static ApiCheckBox add_checkbox(const std::string &text);

    static std::shared_ptr<ApiDropDown> add_dropdown(uint64_t parent_id, const std::vector<std::string> &options);
    static std::shared_ptr<ApiDropDown> add_dropdown(const std::vector<std::string> &options);

  public: // Raw pointers (escape hatch)
    static Globals *raw_globals();
    static Engine *raw_engine();
    static Scene003 *raw_scene();
    static WorldNode *raw_world_root();
    static PlayerUpdateCache *raw_player_update_cache();
    static NpcUpdateCache *raw_npc_update_cache();
    static Player *raw_self();
    static std::vector<Player *> raw_players();
    static std::vector<Npc *> raw_npcs();
    static SocialCache *raw_social_cache();
    static bool raw_is_friend(const Player *player);
    static WorldSettingCache *raw_world_setting_cache();
    static WidgetCache *raw_widget_cache();
    static LocalPlayerVariables *raw_local_player_variables();
    static Menu *raw_menu();
    static SDL_Window *raw_sdl_window();

  public: // World
    static GameState game_state();
    static bool in_game();
    static uint32_t engine_time();
    static std::string local_name();
    static uint32_t run_energy();
    static std::span<const std::string_view> skill_names();
    static std::string_view skill_name(uint32_t index);
    static std::vector<Skill> skills();
    static std::optional<Skill> skill(uint32_t index);

    static std::optional<ApiPlayer> self();
    static std::vector<ApiPlayer> players(std::function<bool(const ApiPlayer &)> conditional = [](const ApiPlayer &)
    {
      return true;
    });
    static std::vector<ApiNpc> npcs(std::function<bool(const ApiNpc &)> conditional = [](const ApiNpc &)
    {
      return true;
    });
    static std::vector<ApiEntity> scene_entities(std::optional<EntityType> type = std::nullopt);

    static uint32_t get_world_setting(uint32_t id);

    static std::optional<ApiItemContainer> get_item_container(uint32_t id, uint16_t parent_widget = 0xffff, uint16_t child_widget = 0xffff);
    static std::optional<ApiItemContainer> get_inventory();
    static std::optional<ApiItemContainer> get_bank();
    static bool has_selected_item();

    static std::vector<FriendView> friends();
    static std::vector<std::string> ignored();
    static bool is_friend(const std::string &name);

    static bool menu_open();
    static std::optional<ApiWidget> widget(uint16_t parent, uint16_t child);
    static std::optional<Vec2<int32_t>> window_size();
    static std::optional<Vec2<float>> world_to_screen(const Vec3<float> &scene);

  public: // Menu
    static FnMenuActionHandler get_menu_action_handler(MenuActionType type, uint32_t idx = 0);
    static void perform_menu_action(FnMenuActionHandler handler, const MenuActionArgs &args);
    static void select_item(uint16_t parent_widget, uint16_t child_widget, int32_t slot);
    static void override_current_menu_action(FnMenuActionHandler handler, const MenuActionArgs &args, bool bypass = false);
    static void walk(uint32_t tile_x, uint32_t tile_y);
    static void interact_object(uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option = 0);
    static void interact_npc(uint32_t server_index, uint32_t option = 0);
    static void interact_widget(uint16_t parent, uint16_t child, int32_t slot, uint32_t option = 0, uint32_t handler = 0);
    static void override_walk(uint32_t tile_x, uint32_t tile_y);
    static void override_object(uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option = 0);
    static void override_npc(uint32_t server_index, uint32_t option = 0);
    static void override_widget(uint16_t parent, uint16_t child, int32_t slot, uint32_t option = 0, uint32_t handler = 0);

  public: // Events
    static uint64_t on_tick(std::function<void()> f);
    static uint64_t on_menu_action(std::function<void(MenuActionEventArgs *)> f);
    static uint64_t on_menu_opened(std::function<void(bool)> f);
    static uint64_t on_world_setting_changed(std::function<void(uint32_t, uint32_t)> f);
    static uint64_t on_item_changed(std::function<void(uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t)> f);
    static uint64_t on_new_chat_message(std::function<void(const std::string &, const std::string &, const std::string &)> f);

  public: // Inter-plugin
    static void expose(const std::string &name, void *function);
    static void *exposed(const std::string &name);

    template <typename T>
    static T exposed(const std::string &name)
    {
      return reinterpret_cast<T>(exposed(name));
    }

  public: // Utils
    static void log(const std::string &s);

    template <typename... Args>
    static void log(std::format_string<Args...> fmt, Args &&...args)
    {
      log(std::format(fmt, std::forward<Args>(args)...));
    }
  };
} // namespace crs
