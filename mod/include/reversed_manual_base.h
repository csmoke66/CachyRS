#pragma once
#include <cstdint>
#include "reversed_util.h"
#include "math.h"

#pragma pack(push, 1)
template<typename T>
struct JArray
{
    // 0x0
    T* begin;
    // 0x8
    T* end;
    // 0x10

	FINLINE size_t size()
	{
		auto raw_distance = (uint64_t)end - (uint64_t)begin;
		return raw_distance / sizeof(T);
	}

	FINLINE bool is_valid(size_t idx)
	{
		if (begin == end)
		{
			return false;
		}
		
		return idx < size();
	}

	FINLINE T* reference(size_t idx)
	{
		if (!is_valid(idx))
		{
			return nullptr;
		}

		return &begin[idx];
	}
};

// this is actually a typical C++ STL string
class JString
{
public:
	union
	{
		char data[0x17];
		struct
		{
			char* data_ptr;
			uint8_t len1;
			PAD(0x7);
			uint8_t len2;
			PAD(0x6);
		};
	};
	union
	{
		uint8_t remaining_bytes;
		uint8_t flag;
	};

public:
	FINLINE char* c_str()
	{
		if (flag == 0x80)
		{
			return data_ptr;
		}
		else
		{
			return data;
		}
	}
};
static_assert(sizeof(JString) == 0x18, INVALID_SIZE);

#pragma pack(pop)