#pragma once
#include <EGL/egl.h>
#include <SDL2/SDL.h>

#include <cstdint>
#include <type_traits>
#include <string>

#include "util.h"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Winvalid-offsetof"

struct SDL_SysWMinfo;

namespace crs
{
#include "reversed_util.h"
#include "reversed_enum.h"
#include "reversed_manual_base.h"
#include "reversed_fwd_decl.h"
#include "reversed_world_setting.h"
#include "reversed_generated.h"
#include "reversed_social.h"
#include "reversed_render.h"
#include "reversed_cache.h"
#include "reversed_entity_update_cache.h"
#include "reversed_entity.h"
#include "reversed_local_player.h"
#include "reversed_menu.h"
#include "reversed_scene.h"
#include "reversed_widget.h"
#include "reversed_item_cache.h"
#include "reversed_linux.h"
#include "reversed_fn_decl.h"
#include "reversed_traits.h"
}

#pragma GCC diagnostic pop