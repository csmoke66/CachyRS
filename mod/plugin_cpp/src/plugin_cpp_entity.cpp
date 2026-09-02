#include "plugin_cpp.h"

namespace crs
{
    ApiEntity::ApiEntity(Entity *entity)
    {
        this->entity = entity;
    }

    ApiEntity::ApiEntity(const ApiEntity &o)
    {
        this->entity = o.entity;
    }

    ApiNamedEntity::ApiNamedEntity(NamedEntity *named) : ApiEntity(named)
    {
        this->named = named;
    }

    ApiNamedEntity::ApiNamedEntity(const ApiNamedEntity &o) : ApiEntity(o)
    {
        this->named = o.named;
    }

    int32_t ApiNamedEntity::server_index() const
    {
        if (!named)
        {
            return -1;
        }

        return named->server_index;
    }

    std::string ApiNamedEntity::name() const
    {
        if (!named)
        {
            return "INVALID";
        }

        return named->name.str();
    }

    Vec3<float> ApiNamedEntity::scene_position() const
    {
        if (!named)
        {
            return {0.f, 0.f, 0.f};
        }

        return named->position;
    }

    bool ApiNamedEntity::animation_playing() const
    {
        if (!named)
        {
            return false;
        }

        return named->animation_queue.begin != named->animation_queue.end;
    }

    int32_t ApiNamedEntity::animation_id() const
    {
        if (!named)
        {
            return -1;
        }

        if (named->animation_queue.begin == named->animation_queue.end)
        {
            return -1;
        }

        return (int32_t)*named->animation_queue.begin;
    }

    ApiPlayer::ApiPlayer(Player *player) : ApiNamedEntity(player)
    {
        this->player = player;
    }

    ApiPlayer::ApiPlayer(const ApiPlayer &o) : ApiNamedEntity(o)
    {
        this->player = o.player;
    }

    ApiPlayer ApiPlayer::invalid()
    {
        return ApiPlayer(nullptr);
    }

    ApiNpc::ApiNpc(Npc *npc) : ApiNamedEntity(npc)
    {
        this->npc = npc;
    }

    ApiNpc::ApiNpc(const ApiNpc &o) : ApiNamedEntity(o)
    {
        this->npc = o.npc;
    }
}