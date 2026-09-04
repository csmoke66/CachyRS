#pragma once
#include <game_events.h>
#include <plugin.h>
#include <reversed/reversed.h>

#include <functional>
#include <map>
#include <vector>

namespace crs
{
  class ApiEntity
  {
  private:
    Entity *entity;

  public:
    ApiEntity(Entity *entity);
    ApiEntity(const ApiEntity &o);
  };

  class ApiNamedEntity : public ApiEntity
  {
  private:
    NamedEntity *named;

  public:
    ApiNamedEntity(NamedEntity *named);
    ApiNamedEntity(const ApiNamedEntity &o);

  public:
    int32_t server_index() const;
    std::string name() const;
    Vec3<float> scene_position() const;
    Vec2<uint32_t> tile_position() const;
    bool animation_playing() const;
    int32_t animation_id() const;
  };

  class ApiPlayer : public ApiNamedEntity
  {
  private:
    Player *player;

  public:
    ApiPlayer(Player *player);
    ApiPlayer(const ApiPlayer &o);

  public:
    static ApiPlayer invalid();
  };

  class ApiNpc : public ApiNamedEntity
  {
  private:
    Npc *npc;

  public:
    ApiNpc(Npc *player);
    ApiNpc(const ApiNpc &o);
  };

  template <typename T>
  class ApiEventList
  {
  private:
    uint64_t token;
    std::map<uint64_t, T> functions;

  public:
    FINLINE uint64_t reg(T function)
    {
      auto t = token++;
      functions[t] = function;
      return t;
    }

  public:
    FINLINE void iterate(std::function<void(T &)> iterator)
    {
      for (auto [k, v] : functions)
      {
        iterator(v);
      }
    }
  };

  class ApiComponent
  {
  protected:
    PluginApi api;
    uint64_t id;

  public:
    ApiComponent(PluginApi api, uint64_t id);
    ApiComponent(const ApiComponent &o);

  public:
    uint64_t get_id();

  public:
    void set_visible(bool visible);
  };

  class ApiLabel : public ApiComponent
  {
  public:
    ApiLabel(PluginApi api, uint64_t id);
    ApiLabel(const ApiLabel &o);
  };

  class ApiHr : public ApiComponent
  {
  public:
    ApiHr(PluginApi api, uint64_t id);
    ApiHr(const ApiHr &o);
  };

  class ApiCheckBox : public ApiComponent
  {
  public:
    ApiCheckBox();
    ApiCheckBox(PluginApi api, uint64_t id);
    ApiCheckBox(const ApiCheckBox &o);

  public:
    bool is_checked();
  };

  class ApiDropDown
  {
  private:
    PluginApi api;
    uint64_t id;

  private:
    int32_t selected = 0;
    std::vector<std::function<void(int)>> change_handlers;

  public:
    ApiDropDown();
    ApiDropDown(PluginApi api, uint64_t id);
    ApiDropDown(const ApiDropDown &o);

  public:
    void fire_changed(int32_t idx);

  public:
    void on_changed(std::function<void(int32_t)> f);

  public:
    int get_selected();
    bool is_selected(int32_t index);
  };

  class ApiContainer : public ApiComponent
  {
  public:
    ApiContainer();
    ApiContainer(PluginApi api, uint64_t id);
    ApiContainer(const ApiContainer &o);

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
    uint16_t parent_widget;
    uint16_t child_widget;
    int32_t slot;
    int32_t id;
    int32_t amount;

  public:
    ApiItem(uint16_t parent_widget, uint16_t child_widget, int32_t slot, int32_t id, int32_t amount);
    ApiItem(const ApiItem &o);

  public:
    MenuActionArgs create_menu_action_args(int index);

  public:
    int32_t get_id();
    int32_t get_amount();
    int32_t get_slot();
  };

  class ApiItemContainer
  {
  private:
    uint32_t capacity;
    std::vector<ApiItem> items;

  public:
    ApiItemContainer();
    ApiItemContainer(uint32_t capacity, const std::vector<ApiItem> &items);
    ApiItemContainer(const ApiItemContainer &o);

  public:
    std::optional<ApiItem> first(std::function<bool(ApiItem &)> conditional);
    size_t count(std::function<bool(ApiItem &)> conditional = [](auto item)
    {
      return true;
    });
    bool contains(std::function<bool(ApiItem &)> conditional);
    bool is_full();
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
    static void init(crs::InitType type, Plugin *plugin, std::function<void()> first_initializer, std::function<void()> initializer);

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

  public: // Raw game data
    static Globals *raw_globals();
    static Engine *raw_engine();
    static PlayerUpdateCache *raw_player_update_cache();
    static NpcUpdateCache *raw_npc_update_cache();
    static Player *raw_self();
    static std::vector<Player *> raw_players();
    static std::vector<Npc *> raw_npcs();
    static SocialCache *raw_social_cache();
    static bool raw_is_friend(const crs::Player *player);
    static WorldSettingCache *raw_world_setting_cache();
    static WidgetCache *raw_widget_cache();

  public: // API game data
    // players
    static ApiPlayer self();
    static std::vector<ApiPlayer> players(std::function<bool(ApiPlayer &)> conditional = [](ApiPlayer &)
    {
      return true;
    });

    // npcs
    static std::vector<ApiNpc> npcs(std::function<bool(ApiNpc &)> conditional = [](ApiNpc &)
    {
      return true;
    });

    // world settings
    static uint32_t get_world_setting(uint32_t id);

    // item containers
    static std::optional<ApiItemContainer> get_item_container(uint32_t id, uint16_t parent_widget = 0xffff, uint16_t child_widget = 0xffff);
    static std::optional<ApiItemContainer> get_inventory();
    static bool has_selected_item();

    // menu actions
    static FnMenuActionHandler get_menu_action_handler(MenuActionType type, uint32_t idx = 0);
    static void perform_menu_action(FnMenuActionHandler handler, const MenuActionArgs& args);
    // This should be safe from botting risks as it only modifies internal 
    // client state, and does not send any packets.
    static void select_item(uint16_t parent_widget, uint16_t child_widget, int32_t slot);
    // Utility for overriding the next menu action event.
    static void override_current_menu_action(FnMenuActionHandler handler, const MenuActionArgs& args, bool bypass = false);

  public: // C++ event handling
    static uint64_t on_tick(std::function<void()> f);
    static uint64_t on_menu_action(std::function<void(MenuActionEventArgs *)> f);
    static uint64_t on_world_setting_changed(std::function<void(uint32_t, uint32_t)> f);

  public: // Utils
    static void log(const std::string &s);
  };
} // namespace crs