// this is a temp file for things that need to go in the updater

#include "reversed_generated.h"
#include "reversed_manual_base.h"
#include "reversed_util.h"

static_assert(true);

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wuninitialized"
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#pragma clang diagnostic ignored "-Wunused-private-field"
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#pragma clang diagnostic ignored "-Wmismatched-tags"
#pragma clang diagnostic ignored "-Wpadded"

#pragma pack(push, 1)

class Obj1 : public Entity
{
public:
  // 0x68
  PAD(0x48);
  // 0xb0
  uint32_t id_1;
  // 0xb4
  PAD(0x4);
  // 0xb8
  CacheBuffer<void, ObjectCacheDesc> cache_buffer;
  // 0xc8
  PAD(0xac);
  // 0x174
  crs::Vec2<uint32_t> tile_position;
  // 0x17c
  PAD(0x34);
  // 0x1b0
  uint32_t id_2;
};
static_assert(off(Obj1, id_1) == 0xb0, INVALID_OFFSET);
static_assert(off(Obj1, cache_buffer) == 0xb8, INVALID_OFFSET);
static_assert(off(Obj1, tile_position) == 0x174, INVALID_OFFSET);
static_assert(off(Obj1, id_2) == 0x1b0, INVALID_OFFSET);

class Obj2 : public Entity
{
public:
  // 0x68
  PAD(0x38);
  // 0xa0
  crs::Vec2<uint32_t> tile_position;
  // 0xa8
  CacheBuffer<void, ObjectCacheDesc> cache_buffer;
  // 0xb8
  PAD(0x118);
  // 0x1d0
  crs::Vec2<uint32_t> tile_position_2;
};
static_assert(off(Obj2, tile_position) == 0xa0, INVALID_OFFSET);
static_assert(off(Obj2, cache_buffer) == 0xa8, INVALID_OFFSET);
static_assert(off(Obj2, tile_position_2) == 0x1d0, INVALID_OFFSET);

#pragma pack(pop)

#pragma clang diagnostic pop
