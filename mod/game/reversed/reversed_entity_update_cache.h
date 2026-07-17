#pragma once

#pragma pack(push, 1)
struct PlayerUpdate
{
	// 0x0
	PAD(0x38);
	// 0x38
	const NamedEntity* entity;
	// 0x40
};

struct PlayerUpdateCache
{
	// 0x0
	PAD(0x10);
	// 0x10
	const JArray<const PlayerUpdate*> updates;
	// 0x20
};

struct NpcUpdate
{
	uint32_t id;
	// 0x4
	PAD(0x4);
	// 0x8
	const ObjectHeader<NamedEntity>* entity_header;
	// 0x10
	NamedEntity* entity;
	// 0x18
	PAD(0x8);
	// 0x20
};

#pragma pack(pop)