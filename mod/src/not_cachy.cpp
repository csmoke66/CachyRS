#include "not_cachy.h"
#include "cachy.h"

Scene003* NotCachyRS::scene_003()
{
    auto globals = RS.get_globals();
    auto engine = globals->engine;
    if (!engine)
    {
        return nullptr;
    }

    auto scene_001 = engine->scene_001;
    if (!scene_001)
    {
        return nullptr;
    }

    auto scene_002 = scene_001->scene_002.reference(scene_001->scene_index);
    return scene_002->scene_003;
}

NotCachyRS NRS;