#pragma once

#pragma pack(push, 1)
struct Item
{
	// 0x0
	const int32_t id;
	// 0x4
	const int32_t amount;
	// 0xc

	Item();
	Item(const Item &o);
};

template <typename T>
struct JArray
{
	// 0x0
	T *begin;
	// 0x8
	T *end;
	// 0x10

	FINLINE size_t size() const
	{
		auto raw_distance = (uint64_t)end - (uint64_t)begin;
		return raw_distance / sizeof(T);
	}

	FINLINE bool is_valid(size_t idx) const
	{
		if (begin == end)
		{
			return false;
		}

		return idx < size();
	}

	FINLINE T *reference(size_t idx) const
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
		const char data[0x17];
		struct
		{
			char *data_ptr;
			const uint8_t len1;
			PAD(0x7);
			const uint8_t len2;
			PAD(0x6);
		};
	};
	union
	{
		const uint8_t remaining_bytes;
		const uint8_t flag;
	};

public:
	FINLINE const char *c_str() const
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