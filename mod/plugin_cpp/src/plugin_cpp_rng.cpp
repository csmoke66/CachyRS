#include "plugin_cpp.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <random>

namespace crs
{
  namespace
  {
    uint64_t &stored_seed()
    {
      static uint64_t value = 0;
      return value;
    }

    std::minstd_rand &engine()
    {
      // One engine per plugin .so (plugin_cpp is statically linked).
      static std::minstd_rand rng{ 1 };
      static bool seeded = false;
      if (!seeded)
      {
        auto s = Api::mixed_seed();
        stored_seed() = s;
        rng.seed(static_cast<std::minstd_rand::result_type>(s));
        seeded = true;
      }
      return rng;
    }

    std::string env_or(const char *key, const char *fallback)
    {
      const char *v = std::getenv(key);
      return (v && *v) ? std::string(v) : std::string(fallback);
    }
  } // namespace

  uint64_t Api::hash_string(std::string_view value)
  {
    // FNV-1a 64-bit
    uint64_t hash = 14695981039346656037ull;
    for (unsigned char c : value)
    {
      hash ^= c;
      hash *= 1099511628211ull;
    }
    return hash ? hash : 1ull;
  }

  uint64_t Api::mix_seed(uint64_t a, uint64_t b)
  {
    // Boost hash_combine — avoids XOR cancellation when mixing related values.
    a ^= b + 0x9e3779b97f4a7c15ull + (a << 6) + (a >> 2);
    return a ? a : 1ull;
  }

  uint64_t Api::account_seed()
  {
    return hash_string(env_or("JX_CHARACTER_ID", ""));
  }

  uint64_t Api::session_seed()
  {
    return hash_string(env_or("JX_SESSION_ID", ""));
  }

  uint64_t Api::mixed_seed()
  {
    return mix_seed(account_seed(), session_seed());
  }

  uint64_t Api::day_seed()
  {
    auto days = std::chrono::duration_cast<std::chrono::days>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count();
    return static_cast<uint64_t>(days) ? static_cast<uint64_t>(days) : 1ull;
  }

  uint64_t Api::seed(SeedKind kind)
  {
    switch (kind)
    {
    case SeedKind::account:
      return account_seed();
    case SeedKind::session:
      return session_seed();
    case SeedKind::mixed:
    default:
      return mixed_seed();
    }
  }

  void Api::rng_reseed(uint64_t seed)
  {
    auto s = seed ? seed : 1ull;
    stored_seed() = s;
    engine().seed(static_cast<std::minstd_rand::result_type>(s));
  }

  void Api::rng_reseed(SeedKind kind)
  {
    rng_reseed(seed(kind));
  }

  uint64_t Api::rng_seed_value()
  {
    return stored_seed() ? stored_seed() : mixed_seed();
  }

  uint64_t Api::random_u64(uint64_t lo, uint64_t hi)
  {
    if (lo > hi)
    {
      std::swap(lo, hi);
    }
    return std::uniform_int_distribution<uint64_t>(lo, hi)(engine());
  }

  int32_t Api::random_i32(int32_t lo, int32_t hi)
  {
    if (lo > hi)
    {
      std::swap(lo, hi);
    }
    return std::uniform_int_distribution<int32_t>(lo, hi)(engine());
  }

  double Api::random_f64(double lo, double hi)
  {
    if (lo > hi)
    {
      std::swap(lo, hi);
    }
    return std::uniform_real_distribution<double>(lo, hi)(engine());
  }
} // namespace crs
