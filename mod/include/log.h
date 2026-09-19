#pragma once

#include "ui_log.h"

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
      ss << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec;              \
      const auto crs_log_line = ss.str();                                                        \
      std::cout << crs_log_line << std::endl;                                                    \
      std::cout.flush();                                                                         \
      ::crs::ui_log_push(#LVL, crs_log_line);                                                    \
    }                                                                                            \
  }                                                                                              \
  while (0)
#else
#define LOG(LVL, ...)                                                                            \
  do                                                                                             \
  {                                                                                              \
    std::stringstream ss;                                                                        \
    ss << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec;                \
    const auto crs_log_line = ss.str();                                                          \
    std::cout << crs_log_line << std::endl;                                                      \
    std::cout.flush();                                                                           \
    ::crs::ui_log_push(#LVL, crs_log_line);                                                      \
  }                                                                                              \
  while (0)
#endif
} // namespace crs
