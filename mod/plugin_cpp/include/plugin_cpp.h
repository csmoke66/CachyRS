#pragma once
#include "math.h"

#include <cstdint>
#include <format>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
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
  class ApiObject;
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
    virtual Vec2<uint32_t> tile_position() const;
    uint32_t plane() const;
    uint32_t distance_to(Vec2<uint32_t> tile) const;
    uint32_t distance_to(const ApiEntity &other) const;
    bool same_plane(const ApiEntity &other) const;
    bool rendered() const;
    void set_rendered(bool visible);
    std::optional<ApiPlayer> as_player() const;
    std::optional<ApiNpc> as_npc() const;
    std::optional<ApiObject> as_object() const;
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

  class ApiObject : public ApiEntity
  {
  public:
    ApiObject(Entity *entity = nullptr);
    ApiObject(const ApiObject &o) = default;
    ApiObject &operator=(const ApiObject &o) = default;

  public:
    bool is_obj1() const;
    bool is_obj2() const;
    Obj1 *raw_obj1() const;
    Obj2 *raw_obj2() const;
    ObjectCacheDesc *cache() const;

    uint32_t id() const;
    uint32_t interact_id() const;
    bool matches_id(uint32_t object_id) const;
    bool matches_name(std::string_view object_name) const;
    std::string name() const;
    Vec2<uint32_t> tile_position() const override;

    std::string option_text(uint32_t index) const;
    std::vector<std::pair<uint32_t, std::string>> options() const;
    std::optional<uint32_t> option_index(std::string_view option_name) const;

    void interact(uint32_t option = 0) const;
    bool interact(std::string_view option_name) const;
  };

  template <typename T>
  class ApiEventList
  {
  private:
    struct Entry
    {
      T fn{};
      std::function<bool()> when{};
    };

    uint64_t token = 0;
    std::map<uint64_t, Entry> functions;

  public:
    FINLINE uint64_t reg(T function, std::function<bool()> when = {})
    {
      auto id = token++;
      functions[id] = Entry{ std::move(function), std::move(when) };
      return id;
    }

  public:
    FINLINE void iterate(std::function<void(T &)> callback)
    {
      for (auto &[id, entry] : functions)
      {
        if (entry.when && !entry.when())
        {
          continue;
        }
        callback(entry.fn);
      }
    }
  };

  using EventGate = std::function<bool()>;

  class ApiComponent
  {
  protected:
    PluginHostApi api{};
    uint64_t id = static_cast<uint64_t>(-1);

  public:
    ApiComponent(PluginHostApi api, uint64_t id);
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
    ApiLabel(PluginHostApi api, uint64_t id);
    ApiLabel(const ApiLabel &o) = default;
    ApiLabel &operator=(const ApiLabel &o) = default;

  public:
    void set_text(const std::string &text);
  };

  class ApiHr : public ApiComponent
  {
  public:
    ApiHr(PluginHostApi api, uint64_t id);
    ApiHr(const ApiHr &o) = default;
    ApiHr &operator=(const ApiHr &o) = default;
  };

  class ApiCheckBox : public ApiComponent
  {
  public:
    ApiCheckBox();
    ApiCheckBox(PluginHostApi api, uint64_t id);
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
    ApiDropDown(PluginHostApi api, uint64_t id);
    ApiDropDown(const ApiDropDown &o) = default;
    ApiDropDown &operator=(const ApiDropDown &o) = default;

  public:
    void fire_changed(int32_t idx);
    void on_changed(std::function<void(int32_t)> handler);
    int32_t get_selected() const;
    bool is_selected(int32_t index) const;
    void set_selected(int32_t index);
    void set_items(const std::vector<std::string> &options);
  };

  class ApiButton : public ApiComponent
  {
  private:
    std::vector<std::function<void()>> click_handlers;

  public:
    ApiButton();
    ApiButton(PluginHostApi api, uint64_t id);
    ApiButton(const ApiButton &o) = default;
    ApiButton &operator=(const ApiButton &o) = default;

  public:
    void set_text(const std::string &text);
    void fire_clicked();
    void on_click(std::function<void()> handler);
  };

  struct GraphMapNodeView
  {
    uint32_t id = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint8_t flags = 0; // bit0: has edge to another plane
  };

  struct GraphMapEdgeView
  {
    uint32_t from = 0;
    uint32_t to = 0;
    uint8_t kind = 0;
  };

  struct GraphMapObjectView
  {
    uint32_t object_id = 0;
    uint32_t x = 0;
    uint32_t y = 0;
    uint32_t option = 0;
    std::string name;
  };

  enum class GraphMapPrimitiveKindView : uint8_t
  {
    line = 0,
    disc = 1,
  };

  struct GraphMapPrimitiveView
  {
    GraphMapPrimitiveKindView kind = GraphMapPrimitiveKindView::line;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
    float thickness = 1.f;
    float x0 = 0.f;
    float y0 = 0.f;
    float x1 = 0.f;
    float y1 = 0.f;
  };

  class ApiGraphMap : public ApiComponent
  {
  private:
    std::vector<std::function<void(uint32_t)>> select_handlers;
    std::vector<std::function<void(uint32_t, uint32_t)>> link_handlers;
    std::vector<std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)>> object_link_handlers;
    std::vector<std::function<void(uint32_t, uint32_t)>> background_handlers;
    std::vector<std::function<void(uint32_t, uint32_t, uint32_t, int32_t)>> context_handlers;
    std::vector<std::function<void(uint32_t)>> zoom_handlers;

  public:
    ApiGraphMap();
    ApiGraphMap(PluginHostApi api, uint64_t id);
    ApiGraphMap(const ApiGraphMap &o) = default;
    ApiGraphMap &operator=(const ApiGraphMap &o) = default;

  public:
    void set_view(uint32_t center_x, uint32_t center_y, uint32_t radius_tiles,
                  uint32_t selected_id, const std::vector<GraphMapNodeView> &nodes,
                  const std::vector<GraphMapEdgeView> &edges,
                  const std::vector<GraphMapObjectView> &objects = {});
    void set_primitives(const std::vector<GraphMapPrimitiveView> &primitives);
    void fire_select(uint32_t node_id);
    void fire_link(uint32_t from, uint32_t to);
    void fire_object_link(uint32_t from_vertex, uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option);
    void fire_background(uint32_t tile_x, uint32_t tile_y);
    void fire_context(uint32_t action, uint32_t tile_x, uint32_t tile_y, int32_t vertex_id);
    void fire_zoom(uint32_t radius_tiles);
    void on_select(std::function<void(uint32_t)> handler);
    void on_link(std::function<void(uint32_t, uint32_t)> handler);
    void on_object_link(std::function<void(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)> handler);
    void on_background(std::function<void(uint32_t, uint32_t)> handler);
    void on_context(std::function<void(uint32_t, uint32_t, uint32_t, int32_t)> handler);
    void on_zoom(std::function<void(uint32_t)> handler);
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
    std::shared_ptr<DropDownContentChanger *> self;

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
    ApiContainer(PluginHostApi api, uint64_t id);
    ApiContainer(const ApiContainer &o) = default;
    ApiContainer &operator=(const ApiContainer &o) = default;

  public:
    ApiContainer add_container();
    ApiContainer add_row();
    ApiLabel add_label(const std::string &text);
    ApiHr add_hr();
    ApiCheckBox add_checkbox(const std::string &text);
    std::shared_ptr<ApiDropDown> add_dropdown(std::vector<std::string> options);
    std::shared_ptr<ApiButton> add_button(const std::string &text);
    std::shared_ptr<ApiGraphMap> add_graph_map();
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
    static std::string version();
    static void init();
    static void init_ui();
  };

  class Api
  {
  public:
    static void init(crs::InitType type, Plugin *loaded, std::function<void()> first_initializer, std::function<void()> initializer);
    static void force_on();
    static bool enabled();
    static std::string configuration_dir();

  public: // UI
    static uint64_t root_plugin_component_id();

    static ApiContainer add_container(uint64_t parent_id);
    static ApiContainer add_container();
    static ApiContainer add_row(uint64_t parent_id);
    static ApiContainer add_row();

    static ApiLabel add_label(uint64_t parent_id, const std::string &text);
    static ApiLabel add_label(const std::string &text);

    static ApiHr add_hr(uint64_t parent_id);
    static ApiHr add_hr();

    static ApiCheckBox add_checkbox(uint64_t parent_id, const std::string &text);
    static ApiCheckBox add_checkbox(const std::string &text);

    static std::shared_ptr<ApiDropDown> add_dropdown(uint64_t parent_id, const std::vector<std::string> &options);
    static std::shared_ptr<ApiDropDown> add_dropdown(const std::vector<std::string> &options);

    static std::shared_ptr<ApiButton> add_button(uint64_t parent_id, const std::string &text);
    static std::shared_ptr<ApiButton> add_button(const std::string &text);

    static std::shared_ptr<ApiGraphMap> add_graph_map(uint64_t parent_id);
    static std::shared_ptr<ApiGraphMap> add_graph_map();

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
    static bool logged_in();
    static bool when_always();
    static bool when_in_game();
    static bool when_logged_in();
    static uint32_t engine_time();
    static std::string local_name();
    static uint32_t run_energy();
    static std::span<const std::string_view> skill_names();
    static std::string_view skill_name(uint32_t index);
    static std::vector<Skill> skills();
    static std::optional<Skill> skill(uint32_t index);

    static std::optional<ApiPlayer> self();
    static std::optional<Vec2<uint32_t>> self_tile();
    static std::vector<ApiPlayer> players(std::function<bool(const ApiPlayer &)> conditional = [](const ApiPlayer &)
    {
      return true;
    });
    static std::vector<ApiNpc> npcs(std::function<bool(const ApiNpc &)> conditional = [](const ApiNpc &)
    {
      return true;
    });
    static std::vector<ApiObject> objects(std::function<bool(const ApiObject &)> conditional = [](const ApiObject &)
    {
      return true;
    });
    static std::vector<ApiEntity> scene_entities(std::optional<EntityType> type = std::nullopt);

    static std::optional<ApiPlayer> find_player(std::function<bool(const ApiPlayer &)> conditional);
    static std::optional<ApiNpc> find_npc(std::function<bool(const ApiNpc &)> conditional);
    static std::optional<ApiObject> find_object(std::function<bool(const ApiObject &)> conditional);

    static std::optional<ApiPlayer> closest_player(std::function<bool(const ApiPlayer &)> conditional = [](const ApiPlayer &)
    {
      return true;
    },
                                                  uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiPlayer> closest_player(Vec2<uint32_t> from, std::function<bool(const ApiPlayer &)> conditional = [](const ApiPlayer &)
    {
      return true;
    },
                                                  uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiNpc> closest_npc(std::function<bool(const ApiNpc &)> conditional = [](const ApiNpc &)
    {
      return true;
    },
                                              uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiNpc> closest_npc(Vec2<uint32_t> from, std::function<bool(const ApiNpc &)> conditional = [](const ApiNpc &)
    {
      return true;
    },
                                              uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiObject> closest_object(std::function<bool(const ApiObject &)> conditional = [](const ApiObject &)
    {
      return true;
    },
                                                  uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiObject> closest_object(Vec2<uint32_t> from, std::function<bool(const ApiObject &)> conditional = [](const ApiObject &)
    {
      return true;
    },
                                                  uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiObject> closest_object_id(uint32_t object_id, uint32_t max_dist = UINT32_MAX);
    static std::optional<ApiObject> closest_object_id(Vec2<uint32_t> from, uint32_t object_id, uint32_t max_dist = UINT32_MAX);

    static uint32_t chebyshev(Vec2<uint32_t> a, Vec2<uint32_t> b);
    static uint32_t chebyshev(uint32_t ax, uint32_t ay, uint32_t bx, uint32_t by);
    static bool same_plane(const ApiEntity &entity);

    static uint32_t get_world_setting(uint32_t id);

    static std::optional<ApiItemContainer> get_item_container(uint32_t id, uint16_t parent_widget = 0xffff, uint16_t child_widget = 0xffff, uint32_t fallback_capacity = 0);
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
    static void walk(Vec2<uint32_t> tile);
    static void interact_object(uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option = 0);
    static void interact_npc(uint32_t server_index, uint32_t option = 0);
    static void interact_widget(uint16_t parent, uint16_t child, int32_t slot, uint32_t option = 0, uint32_t handler = 0);
    static void override_walk(uint32_t tile_x, uint32_t tile_y);
    static void override_object(uint32_t object_id, uint32_t tile_x, uint32_t tile_y, uint32_t option = 0);
    static void override_npc(uint32_t server_index, uint32_t option = 0);
    static void override_widget(uint16_t parent, uint16_t child, int32_t slot, uint32_t option = 0, uint32_t handler = 0);

  public: // Events
    static uint64_t on_tick(std::function<void()> f, EventGate when = {});
    static uint64_t on_menu_action(std::function<void(MenuActionEventArgs *)> f, EventGate when = {});
    static uint64_t on_menu_opened(std::function<void(bool)> f, EventGate when = {});
    static uint64_t on_world_setting_changed(std::function<void(uint32_t, uint32_t)> f, EventGate when = {});
    static uint64_t on_item_changed(std::function<void(uint32_t, uint32_t, int32_t, int32_t, int32_t, int32_t, int32_t)> f, EventGate when = {});
    static uint64_t on_new_chat_message(std::function<void(const std::string &, const std::string &, const std::string &)> f, EventGate when = {});

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

    static uint64_t hash_string(std::string_view value);
    static uint64_t mix_seed(uint64_t a, uint64_t b);

    static uint64_t account_seed(); // JX_CHARACTER_ID
    static uint64_t session_seed(); // JX_SESSION_ID
    static uint64_t mixed_seed();   // account mixed with session
    static uint64_t day_seed();     // UTC days since epoch

    enum class SeedKind : uint8_t
    {
      account = 0,
      session = 1,
      mixed = 2,
    };

    static uint64_t seed(SeedKind kind);

    static void rng_reseed(uint64_t seed);
    static void rng_reseed(SeedKind kind);
    static uint64_t rng_seed_value();

    static uint64_t random_u64(uint64_t lo, uint64_t hi);
    static int32_t random_i32(int32_t lo, int32_t hi);
    static double random_f64(double lo = 0.0, double hi = 1.0);
  };
} // namespace crs
