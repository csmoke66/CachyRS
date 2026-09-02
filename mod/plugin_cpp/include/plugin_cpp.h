#include <reversed/reversed.h>
#include <plugin.h>

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

    class ApiLabel
    {
    private:
        PluginApi api;
        uint64_t id;

    public:
        ApiLabel(PluginApi api, uint64_t id);
        ApiLabel(const ApiLabel &o);
    };

    class ApiHr
    {
    private:
        PluginApi api;
        uint64_t id;

    public:
        ApiHr(PluginApi api, uint64_t id);
        ApiHr(const ApiHr &o);
    };

    class ApiCheckBox
    {
    private:
        PluginApi api;
        uint64_t id;

    public:
        ApiCheckBox();
        ApiCheckBox(PluginApi api, uint64_t id);
        ApiCheckBox(const ApiCheckBox &o);

    public:
        bool is_checked();
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
    private:
        static Plugin *plugin;
        static PluginApi api;

    public:
        static ApiEventList<std::function<void()>> tick_events;

    public:
        static void init(crs::InitType type, Plugin *plugin, std::function<void()> first_initializer, std::function<void()> initializer);
        
    public:
        static ApiLabel add_label(const std::string &text);
        static ApiHr add_hr();
        static ApiCheckBox add_checkbox(const std::string &text);

    public:
        static Engine *raw_engine();
        static PlayerUpdateCache *raw_player_update_cache();
        static NpcUpdateCache *raw_npc_update_cache();
        static Player *raw_self();
        static std::vector<Player *> raw_players();
        static std::vector<Npc*> raw_npcs();
        static crs::SocialCache *raw_social_cache();
        static bool raw_is_friend(const crs::Player *player);

    public:
        static ApiPlayer self();
        // clang-format off
        static std::vector<ApiPlayer> players(std::function<bool(ApiPlayer&)> conditional = [](ApiPlayer&) { return true; });
        // clang-format on

    public:
        // clang-format off
        static std::vector<ApiNpc> npcs(std::function<bool(ApiNpc&)> conditional = [](ApiNpc&) { return true; });
        // clang-format on

    public:
        static uint64_t on_tick(std::function<void()> f);
    };
}