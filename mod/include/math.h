#pragma once

namespace crs
{
#pragma pack(push, 1)
	union Matrix4x4
	{
		float d2[4][4];
		float flat[16];
	};

	template <typename T>
	struct Vec2
	{
		T x, y;
	};

	template <typename T>
	struct Vec3
	{
		T x, y, z;
	};
#pragma pack(pop)
}