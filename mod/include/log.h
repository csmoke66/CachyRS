#pragma once

#include <iostream>
#include <sstream>
#include <string_view>

namespace crs
{
#ifdef NDEBUG
#define LOG(LVL, ...)                                                                            \
  do                                                                                             \
  {                                                                                              \
    constexpr std::string_view crs_log_lvl = #LVL;                                               \
    if constexpr (crs_log_lvl != "DEBUG")                                                        \
    {                                                                                            \
      std::stringstream ss;                                                                      \
      ss << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
      std::cout << ss.str();                                                                     \
      std::cout.flush();                                                                         \
    }                                                                                            \
  }                                                                                              \
  while (0)
#else
#define LOG(LVL, ...)                                                                          \
  do                                                                                           \
  {                                                                                            \
    std::stringstream ss;                                                                      \
    ss << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
    std::cout << ss.str();                                                                     \
    std::cout.flush();                                                                         \
  }                                                                                            \
  while (0)
#endif
} // namespace crs
