#pragma once
#include "reversed_util.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>

static_assert(true);

namespace jmem_detail
{
  // x86-64 Linux user VA: lower canonical half. Upper half is kernel.
  inline constexpr uintptr_t k_user_va_max = (uintptr_t{1} << 47) - 1;
  inline constexpr uintptr_t k_user_va_min = 0x10000;
  // Hard cap — game collections never need anything near this.
  inline constexpr uint64_t k_max_elements = 90'000;
  inline constexpr uint64_t k_max_string = 1'000'000;

  [[noreturn]] FINLINE void corrupt()
  {
    std::abort();
  }

  FINLINE void check_user_ptr(const void *p)
  {
    const auto u = reinterpret_cast<uintptr_t>(p);
    if (u == 0)
    {
      return;
    }
    if (u < k_user_va_min || u > k_user_va_max)
    {
      corrupt();
    }
  }

  FINLINE void check_nonnull_user_ptr(const void *p)
  {
    if (!p)
    {
      corrupt();
    }
    check_user_ptr(p);
  }

  template <typename T>
  FINLINE void check_aligned_ptr(const T *p)
  {
    if (!p)
    {
      return;
    }
    if (reinterpret_cast<uintptr_t>(p) % alignof(T) != 0)
    {
      corrupt();
    }
  }

  FINLINE void check_count(uint64_t n)
  {
    if (n > k_max_elements)
    {
      corrupt();
    }
  }
} // namespace jmem_detail

#pragma pack(push, 1)
struct Item
{
  // 0x0
  int32_t id;
  // 0x4
  int32_t amount;
  // 0xc

  Item();
  Item(const Item &o) = default;
  Item &operator=(const Item &o) = default;
};

template <typename T>
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
class JArray
{
  static_assert(!std::is_void_v<T>, "JArray<void> is not supported");

private:
  // 0x0
  uint64_t size_;
  // 0x8
  T *data_;
  // 0x10

public:
  FINLINE void validate() const
  {
    jmem_detail::check_count(size_);
    if (size_ == 0)
    {
      // Empty: data may be null or stale; only reject kernel/non-canonical.
      jmem_detail::check_user_ptr(data_);
      jmem_detail::check_aligned_ptr(data_);
      return;
    }
    jmem_detail::check_nonnull_user_ptr(data_);
    jmem_detail::check_aligned_ptr(data_);
  }

  FINLINE uint64_t size() const
  {
    validate();
    return size_;
  }

  FINLINE bool empty() const
  {
    return size() == 0;
  }

  FINLINE T *data() const
  {
    validate();
    return data_;
  }

  FINLINE T *begin() const
  {
    validate();
    return data_;
  }

  FINLINE T *end() const
  {
    validate();
    return data_ ? data_ + size_ : nullptr;
  }

  FINLINE T *at(uint64_t idx) const
  {
    validate();
    if (idx >= size_)
    {
      return nullptr;
    }
    return data_ + idx;
  }

  template <typename Fn>
  FINLINE void for_each(Fn &&fn) const
  {
    validate();
    for (uint64_t i = 0; i < size_; ++i)
    {
      fn(data_[i]);
    }
  }
};
static_assert(sizeof(JArray<void *>) == 0x10, INVALID_SIZE);

template <typename T>
class JArray2
{
  static_assert(!std::is_void_v<T>, "JArray2<void> is not supported");

private:
  // 0x0
  T *data_;
  // 0x8
  uint64_t size_;
  // 0x10

public:
  FINLINE void validate() const
  {
    jmem_detail::check_count(size_);
    if (size_ == 0)
    {
      jmem_detail::check_user_ptr(data_);
      jmem_detail::check_aligned_ptr(data_);
      return;
    }
    jmem_detail::check_nonnull_user_ptr(data_);
    jmem_detail::check_aligned_ptr(data_);
  }

  FINLINE uint64_t size() const
  {
    validate();
    return size_;
  }

  FINLINE bool empty() const
  {
    return size() == 0;
  }

  FINLINE T *data() const
  {
    validate();
    return data_;
  }

  FINLINE T *begin() const
  {
    validate();
    return data_;
  }

  FINLINE T *end() const
  {
    validate();
    return data_ ? data_ + size_ : nullptr;
  }

  FINLINE T *at(uint64_t idx) const
  {
    validate();
    if (idx >= size_)
    {
      return nullptr;
    }
    return data_ + idx;
  }

  template <typename Fn>
  FINLINE void for_each(Fn &&fn) const
  {
    validate();
    for (uint64_t i = 0; i < size_; ++i)
    {
      fn(data_[i]);
    }
  }
};
static_assert(sizeof(JArray2<void *>) == 0x10, INVALID_SIZE);

template <typename T>
class JVector
{
  static_assert(!std::is_void_v<T>, "JVector<void> is not supported");

private:
  // 0x0
  T *begin_;
  // 0x8
  T *end_;
  // 0x10
  T *max_;
  // 0x18

public:
  FINLINE void validate() const
  {
    jmem_detail::check_user_ptr(begin_);
    jmem_detail::check_user_ptr(end_);
    jmem_detail::check_user_ptr(max_);
    jmem_detail::check_aligned_ptr(begin_);
    jmem_detail::check_aligned_ptr(end_);
    jmem_detail::check_aligned_ptr(max_);

    const auto b = reinterpret_cast<uintptr_t>(begin_);
    const auto e = reinterpret_cast<uintptr_t>(end_);
    const auto m = reinterpret_cast<uintptr_t>(max_);

    if (begin_ == nullptr)
    {
      if (end_ != nullptr || max_ != nullptr)
      {
        jmem_detail::corrupt();
      }
      return;
    }

    if (end_ == nullptr || max_ == nullptr)
    {
      jmem_detail::corrupt();
    }

    // end / max before begin → corruption
    if (e < b || m < b)
    {
      jmem_detail::corrupt();
    }

    // capacity pointer must not be before end
    if (m < e)
    {
      jmem_detail::corrupt();
    }

    const auto span = e - b;
    const auto cap = m - b;
    if (span % sizeof(T) != 0 || cap % sizeof(T) != 0)
    {
      jmem_detail::corrupt();
    }

    jmem_detail::check_count(span / sizeof(T));
    jmem_detail::check_count(cap / sizeof(T));
  }

  FINLINE size_t size() const
  {
    validate();
    if (!begin_)
    {
      return 0;
    }
    return static_cast<size_t>((reinterpret_cast<uintptr_t>(end_) - reinterpret_cast<uintptr_t>(begin_)) / sizeof(T));
  }

  FINLINE bool empty() const
  {
    validate();
    return begin_ == end_;
  }

  FINLINE T *begin() const
  {
    validate();
    return begin_;
  }

  FINLINE T *end() const
  {
    validate();
    return end_;
  }

  FINLINE T *max_ptr() const
  {
    validate();
    return max_;
  }

  FINLINE bool is_valid(size_t idx) const
  {
    return idx < size();
  }

  FINLINE T *reference(size_t idx) const
  {
    if (!is_valid(idx))
    {
      return nullptr;
    }
    return begin_ + idx;
  }

  FINLINE T *front() const
  {
    validate();
    if (begin_ == end_)
    {
      return nullptr;
    }
    return begin_;
  }

  template <typename Fn>
  FINLINE void for_each(Fn &&fn) const
  {
    validate();
    for (T *p = begin_; p != end_; ++p)
    {
      fn(*p);
    }
  }
};
static_assert(sizeof(JVector<void *>) == 0x18, INVALID_SIZE);

// Typical C++ STL string layout (game binary).
class JString
{
private:
  union
  {
    char data_[0x17];
    struct
    {
      // 0x0
      char *data_ptr_;
      // 0x8
      uint8_t len1_;
      // 0x9
      PAD(0x7);
      // 0x10
      uint8_t len2_;
      // 0x11
      PAD(0x6);
      // 0x17
    };
  };
  union
  {
    // 0x17
    uint8_t remaining_bytes_;
    // 0x17
    uint8_t flag_;
  };

  FINLINE bool is_long() const
  {
    return flag_ == 0x80;
  }

  // Inline buffer is data_[0x17]; usable chars before the required NUL.
  static constexpr uint8_t k_sso_max_len = 0x16;

  FINLINE void validate_cstr(const char *s, size_t max_len) const
  {
    jmem_detail::check_nonnull_user_ptr(s);
    for (size_t i = 0; i <= max_len; ++i)
    {
      // Touch each byte; missing NUL within max → corruption.
      if (s[i] == '\0')
      {
        return;
      }
    }
    jmem_detail::corrupt();
  }

public:
  FINLINE void validate() const
  {
    (void)len1_;
    (void)len2_;
    (void)remaining_bytes_;
    if (is_long())
    {
      jmem_detail::check_nonnull_user_ptr(data_ptr_);
      validate_cstr(data_ptr_, jmem_detail::k_max_string);
      return;
    }

    // Short string: require a NUL within the inline buffer.
    validate_cstr(data_, k_sso_max_len);
  }

  FINLINE const char *c_str() const
  {
    validate();
    return is_long() ? data_ptr_ : data_;
  }

  FINLINE std::string str() const
  {
    return std::string(c_str());
  }

  FINLINE bool empty() const
  {
    return c_str()[0] == '\0';
  }
};
static_assert(sizeof(JString) == 0x18, INVALID_SIZE);

template <typename I, uint64_t S, typename T>
struct IdObject
{
  I id;
  PAD(S);
  T body;
};

template <typename T, typename B>
struct TaggedObject
{
  // 0x0
  T *tag;
  // 0x8
  B *body;
  // 0x10
};

#pragma pack(pop)
