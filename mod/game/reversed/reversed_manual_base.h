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

template<typename T>
struct ObjectHeader
{
	// 0x0
	PAD(0x8); // VT
	// 0x8
	uint32_t id1; // could be an id, or a type id
	// 0xc
	uint32_t id2; // could be an id, or a type id
	// 0x10
	T inner;
};

template <typename T>
struct JArray
{
	// 0x0
	uint64_t size;
	// 0x8
	T* data;
	// 0x10
};
static_assert(sizeof(JArray<void*>) == 0x10, INVALID_SIZE);

// do we really need both variations of this..?
template <typename T>
struct JArray2
{
	// 0x0
	T* data;
	// 0x8
	uint64_t size;
	// 0x10
};
static_assert(sizeof(JArray2<void*>) == 0x10, INVALID_SIZE);

template <typename T>
struct JVector
{
	// 0x0
	T *begin;
	// 0x8
	T *end;
	// 0x10
	T *max;
	// 0x18

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
static_assert(sizeof(JVector<void*>) == 0x18, INVALID_SIZE);

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

template<typename I, uint64_t S, typename T>
struct IdObject
{
	I id;
	PAD(S);
	T body;
};

template<typename T, typename B>
struct TaggedObject
{
	// 0x0
	const T* tag;
	// 0x8
	const B* body;
	// 0x10
};

#pragma pack(pop)