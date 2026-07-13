#pragma once
#include "reversed_util.h"

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
private:
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

public:
	// 0x38
	virtual void* get_cache_descriptor_by_index(uint32_t idx, bool unknown) = 0;

public:
	// 0x8
	PAD(0x38);
	// 0x40
	const uint64_t size;
	// 0x48
	const JArray<const CacheBuffer**> data_buffer;
};

struct CacheIndex
{
	// 0x0
	PAD(0x30);
	// 0x30
	const uint32_t current_size;
	// 0x34
	const uint32_t max_size;
	// 0x38
	PAD(0x8);
	// 0x40
	const CacheIndexInner* inner;
	// 0x48
};

struct Cache001
{
	// 0x0
	PAD(0xa8);
	// 0xa8
	const CacheIndex** indices;
	// 0xb0
	PAD(0x110);
	// 0x1c0
	const Cache002* cache_002;
	// 0x1c8
};

#pragma pack(pop)