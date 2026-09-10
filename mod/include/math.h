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

    bool operator==(const Vec2<T> &other) const = default;
  };

  template <typename T>
  struct Vec3
  {
    T x, y, z;

    Vec2<T> to_vec2_xz()
    {
      return { x, z};
    }
    
    bool operator==(const Vec3<T> &other) const = default;
  };

  template <typename T>
  struct Area2
  {
    Vec2<T> begin;
    Vec2<T> end;

    bool is_within(Vec2<T> t, bool exclusive = false)
    {
      if (exclusive)
      {
        return t.x > begin.x &&
               t.y > begin.y &&
               t.x < end.x &&
               t.y < end.y;
      }
      else
      {
        return t.x >= begin.x &&
               t.y >= begin.y &&
               t.x <= end.x &&
               t.y <= end.y;
      }
    }

    bool operator==(const Area2<T> &other) const = default;
  };
#pragma pack(pop)
} // namespace crs