#include "reversed.h"

class NotCachyRS
{
public:
    Scene003* scene_003();
    WorldNode* root_node();
    SDL_Window* sdl_window();
    ItemCache* item_cache();
};

extern NotCachyRS NRS;