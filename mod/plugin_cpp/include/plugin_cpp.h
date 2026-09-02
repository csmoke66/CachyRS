#include <reversed/reversed.h>
#include <plugin.h>
#include <game_events.h>

#include <vector>
#include <map>
#include <functional>

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
        std::shared_ptr<ApiDropDown> add_dropdown(std::initializer_list<std::string> options);
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
        static ApiContainer add_container(uint64_t parent_id);
        static ApiContainer add_container();

        static ApiLabel add_label(uint64_t parent_id, const std::string &text);
        static ApiLabel add_label(const std::string &text);

        static ApiHr add_hr(uint64_t parent_id);
        static ApiHr add_hr();

        static ApiCheckBox add_checkbox(uint64_t parent_id, const std::string &text);
        static ApiCheckBox add_checkbox(const std::string &text);

        static std::shared_ptr<ApiDropDown> add_dropdown(uint64_t parent_id, std::initializer_list<std::string> options);
        static std::shared_ptr<ApiDropDown> add_dropdown(std::initializer_list<std::string> options);

    public: // Raw game data
        static Globals *raw_globals();
        static Engine *raw_engine();
        static PlayerUpdateCache *raw_player_update_cache();
        static NpcUpdateCache *raw_npc_update_cache();
        static Player *raw_self();
        static std::vector<Player *> raw_players();
        static std::vector<Npc *> raw_npcs();
        static crs::SocialCache *raw_social_cache();
        static bool raw_is_friend(const crs::Player *player);

    public: // API game data
        // players
        static ApiPlayer self();
        // clang-format off
        static std::vector<ApiPlayer> players(std::function<bool(ApiPlayer&)> conditional = [](ApiPlayer&) { return true; });
        // clang-format on

        // npcs
        // clang-format off
        static std::vector<ApiNpc> npcs(std::function<bool(ApiNpc&)> conditional = [](ApiNpc&) { return true; });
        // clang-format on

    public: // C++ event handling
        static uint64_t on_tick(std::function<void()> f);
        static uint64_t on_menu_action(std::function<void(MenuActionEventArgs*)> f);

    public: // Utils
        static void log(const std::string &s);
    };
}