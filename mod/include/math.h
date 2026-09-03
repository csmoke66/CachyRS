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

    bool operator==(const Vec2<T>& other) const = default;
  };

  template <typename T>
  struct Vec3
  {
    T x, y, z;

    bool operator==(const Vec3<T>& other) const = default;
  };
#pragma pack(pop)
} // namespace crs