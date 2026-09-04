#pragma once

#pragma pack(push, 1)
struct Friend
{
  JString name;
  // 0x18
  JString previous_name;
  // 0x30
  PAD(0x8);
  // 0x38
  JString world;
  // 0x50
  PAD(0x28);
  // 0x78
};
static_assert(sizeof(Friend) == 0x78, INVALID_SIZE);

struct Ignored
{
  JString name;
  // 0x18
  JString previous_name;
  // 0x30
  PAD(0x20);
  // 0x50
};
struct SocialCache
{
  PAD(0x18);
  // 0x18
  JVector<Friend> friends;
  // 0x30
  JVector<Ignored> ignored;
};
#pragma pack(pop)