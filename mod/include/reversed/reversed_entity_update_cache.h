#pragma once
#include "reversed_util.h"
#include "reversed_fwd_decl.h"

#pragma pack(push, 1)
struct EntityUpdate
{
	// 0x0
	PAD(0x38);
	// 0x38
	const Entity* entity;
	// 0x40
};

struct EntityUpdateCache
{
	// 0x0
	PAD(0x10);
	// 0x10
	const JArray<const EntityUpdate*> updates;
	// 0x20
};

#pragma pack(pop)