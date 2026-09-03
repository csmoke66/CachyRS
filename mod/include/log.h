#pragma once

#include <iostream>
#include <sstream>

namespace crs
{
#define LOG(LVL, ...)                                                                              \
  do                                                                                               \
  {                                                                                                \
    std::stringstream ss;                                                                          \
    ss << "[" << __FUNCTION__ << "][" << #LVL << "] " << __VA_ARGS__ << std::dec << std::endl; \
    std::cout << ss.str();                                                                         \
    std::cout.flush();                                                                             \
  }                                                                                                \
  while (0);
} // namespace crs