#pragma once

#pragma pack(push, 1)
class Cache003
{
public:
	// 0x0
	PAD_VT();
	// 0x8
	PAD_VT();
	// 0x10
	PAD_VT();
	// 0x18
	PAD_VT();
	// 0x20
	PAD_VT();
	// 0x28
	PAD_VT();
	// 0x30
	PAD_VT();
	// 0x38
	virtual void* test(int id, bool unk) = 0;
};

struct Cache002
{
	// 0x0
	PAD(0x40);
	// 0x40
	const Cache003* cache_003;
	// 0x48
};

struct CacheBuffer
{
	// 0x0
	const void* tag;
	// 0x8
	const void* body;
	// 0x10
};

class CacheIndexInner
{
public:
	// 0x0
	PAD_VT();
	// 0x8
	PAD_VT();
	// 0x10
	PAD_VT();
	// 0x18
	PAD_VT();
	// 0x20
	PAD_VT();
	// 0x28
	PAD_VT();
	// 0x30
	PAD_VT();
	// 0x38
	PAD_VT();
	// 0x40
	virtual void* get_cache_descriptor_by_index(uint32_t idx, bool unknown) = 0;

public:
	// 0x8
	PAD(0x38);
	// 0x40
	const uint32_t size;
};
static_assert(off(CacheIndexInner, size) == 0x40, INVALID_OFFSET);

struct CacheIndex
{
	// 0x0
	PAD(0x28);
	// 0x28
	const uint32_t current_size;
	// 0x2c
	const uint32_t max_size;
	// 0x30
	PAD(0x8);
	// 0x38
	const CacheIndexInner* inner;
	// 0x40
};
static_assert(off(CacheIndex, inner) == 0x38, INVALID_OFFSET);

struct Cache001
{
	// 0x0
	PAD(0x80);
	// 0x80
	const CacheIndex** indices;
};

#pragma pack(pop)