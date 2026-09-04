#pragma once

#pragma pack(push, 1)
struct WorldSettingMask
{
  // 0x0
  PAD(0x8);
  // 0x8
  uint32_t world_setting_id;
  // 0xc
};

struct WorldSetting
{
  // 0x8
  uint32_t value;
  // 0xc
  PAD(0x4);
  // 0x10
  uint8_t initialized;
  // 0x11
  PAD(0x17);
  // 0x28
  IdObject<uint32_t, 0x4, WorldSetting> *next;
};
static_assert(off(WorldSetting, value) == 0x0, INVALID_OFFSET);
static_assert(off(WorldSetting, initialized) == 0x8, INVALID_OFFSET);
static_assert(off(WorldSetting, next) == 0x20, INVALID_OFFSET);

// This class actually extends MUCH further, and uses a list much
// further down internally. I'm not sure why.
class WorldSettingCache
{
public:
  // 0x0
  PAD(0x28);
  // 0x28
  IdObject<uint32_t, 0x4, WorldSetting> **vars;
  // 0x30
  uint32_t count;
};
static_assert(sizeof(WorldSettingCache) == 0x34, INVALID_SIZE);
#pragma pack(pop)