#pragma once

#include "log.h"

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

namespace crs::plugin_api_validate
{
  inline constexpr uintptr_t k_user_va_max = (uintptr_t{ 1 } << 47) - 1;
  inline constexpr uintptr_t k_user_va_min = 0x10000;
  inline constexpr uint64_t k_component_id_max = uint64_t{ 1 } << 40;
  inline constexpr size_t k_max_collection = 1'000'000;

  [[noreturn]] inline void fatal(const char *api, const char *reason, uintptr_t detail = 0)
  {
    LOG(ERROR, "plugin API FATAL [" << api << "]: " << reason
                                    << " detail=0x" << std::hex << detail << std::dec);
    std::abort();
  }

  inline void soft_fail(const char *api, const char *reason, uintptr_t detail = 0)
  {
    LOG(ERROR, "plugin API [" << api << "]: " << reason
                              << " detail=0x" << std::hex << detail << std::dec);
  }

  inline bool is_plausible_user_address(const void *p)
  {
    const auto u = reinterpret_cast<uintptr_t>(p);
    if (u == 0)
    {
      return false;
    }
    if (u < k_user_va_min)
    {
      return false;
    }
    if (u > k_user_va_max)
    {
      return false;
    }
    return true;
  }

#ifdef CACHYRS_PLUGIN_API_VALIDATE
  inline bool memory_is_readable(const void *p, size_t len)
  {
    if (!p || len == 0)
    {
      return false;
    }

    static int null_fd = []() -> int
    {
      return ::open("/dev/null", O_WRONLY | O_CLOEXEC);
    }();

    if (null_fd < 0)
    {
      return is_plausible_user_address(p);
    }

    const size_t chunk = static_cast<size_t>(::sysconf(_SC_PAGESIZE));
    const auto *bytes = static_cast<const char *>(p);
    size_t remaining = len;
    size_t offset = 0;
    while (remaining > 0)
    {
      const size_t n = std::min(remaining, chunk);
      errno = 0;
      const ssize_t wrote = ::write(null_fd, bytes + offset, n);
      if (wrote < 0 && errno == EFAULT)
      {
        return false;
      }
      if (wrote < 0)
      {
        return is_plausible_user_address(bytes + offset);
      }
      offset += static_cast<size_t>(wrote);
      remaining -= static_cast<size_t>(wrote);
      if (wrote == 0)
      {
        break;
      }
    }
    return true;
  }
#else
  inline bool memory_is_readable(const void *p, size_t /*len*/)
  {
    return is_plausible_user_address(p);
  }
#endif

  inline void require_readable(const char *api, const char *what, const void *p, size_t len)
  {
    if (!p)
    {
      fatal(api, "null pointer", reinterpret_cast<uintptr_t>(what));
    }
    const auto u = reinterpret_cast<uintptr_t>(p);
    if (u < k_user_va_min || u > k_user_va_max)
    {
      fatal(api, "pointer outside user address space", u);
    }
    if (!memory_is_readable(p, len))
    {
      fatal(api, "pointer not readable (bad mapping)", u);
    }
  }

  inline void optional_readable(const char *api, const void *p, size_t len)
  {
    if (!p)
    {
      return;
    }
    const auto u = reinterpret_cast<uintptr_t>(p);
    if (u < k_user_va_min || u > k_user_va_max)
    {
      fatal(api, "optional pointer outside user address space", u);
    }
    if (!memory_is_readable(p, len))
    {
      fatal(api, "optional pointer not readable", u);
    }
  }

  inline void opaque_cookie_sane(const char *api, const void *p)
  {
    if (!p)
    {
      return;
    }
    const auto u = reinterpret_cast<uintptr_t>(p);
    if (u > k_user_va_max)
    {
      fatal(api, "opaque cookie looks like kernel/non-canonical pointer", u);
    }
  }

  inline void require_c_string(const char *api, const char *what, const char *s)
  {
    if (!s)
    {
      fatal(api, "null C string", reinterpret_cast<uintptr_t>(what));
    }
    require_readable(api, what, s, 1);
#ifdef CACHYRS_PLUGIN_API_VALIDATE
    const auto page = static_cast<size_t>(::sysconf(_SC_PAGESIZE));
    uintptr_t cur = reinterpret_cast<uintptr_t>(s);
    uintptr_t page_end = (cur & ~(uintptr_t{ page } - 1)) + page;
    for (size_t i = 0; i < 1'000'000; ++i)
    {
      if (!memory_is_readable(reinterpret_cast<const void *>(cur), 1))
      {
        fatal(api, "C string walked into unreadable memory", cur);
      }
      if (*reinterpret_cast<const char *>(cur) == '\0')
      {
        return;
      }
      ++cur;
      if (cur >= page_end)
      {
        page_end += page;
      }
    }
    fatal(api, "C string missing NUL terminator within limit", reinterpret_cast<uintptr_t>(s));
#else
    if (std::strlen(s) > 1'000'000)
    {
      fatal(api, "C string absurdly long / missing NUL", reinterpret_cast<uintptr_t>(s));
    }
#endif
  }

  inline void require_fn_ptr(const char *api, const char *what, const void *fn)
  {
    require_readable(api, what, fn, 1);
  }

  inline bool component_id_sane(const char *api, uint64_t id, bool allow_zero = false)
  {
    if (id == 0)
    {
      if (allow_zero)
      {
        return true;
      }
      soft_fail(api, "component id 0", 0);
      return false;
    }
    if (id > k_component_id_max || id > k_user_va_max)
    {
      fatal(api, "component id looks like a corrupted pointer", id);
    }
    return true;
  }

  inline void require_collection_count(const char *api, size_t count, const void *ptr, size_t elem_size)
  {
    if (count > k_max_collection)
    {
      fatal(api, "collection count absurdly large", count);
    }
    if (count > 0 && !ptr)
    {
      fatal(api, "null collection pointer with non-zero count", count);
    }
    if (count > 0)
    {
      require_readable(api, "collection", ptr, count * elem_size);
    }
  }

  template <typename Enum>
  inline void require_enum_range(const char *api, Enum value, Enum max_inclusive)
  {
    using U = std::underlying_type_t<Enum>;
    const auto v = static_cast<U>(value);
    const auto m = static_cast<U>(max_inclusive);
    if (v > m)
    {
      fatal(api, "enum out of range", static_cast<uintptr_t>(v));
    }
  }
} // namespace crs::plugin_api_validate
