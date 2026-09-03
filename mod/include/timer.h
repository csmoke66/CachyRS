#pragma once
#include <chrono>
#include <thread>

class Timer
{
private:
  std::chrono::system_clock::time_point last_time = std::chrono::system_clock::now();

public:
  template <typename _Rep, typename _Period>
  inline bool check(const std::chrono::duration<_Rep, _Period> &tp)
  {
    auto now = std::chrono::system_clock::now();
    auto future = last_time + tp;
    auto passed = now >= future;
    if (passed)
    {
      last_time = now;
    }

    return passed;
  }
};

class Stopwatch
{
private:
  enum class State
  {
    stopped,
    running
  };

private:
  std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  State state = State::running;

public:
  inline auto check() const
  {
    if (state == State::running)
    {
      return std::chrono::steady_clock::now() - start;
    }
    else
    {
      return end - start;
    }
  }

  inline std::chrono::milliseconds check_ms() const
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>(check());
  }

  void reset()
  {
    start = std::chrono::steady_clock::now();
    state = State::running;
  }

  void stop()
  {
    end = std::chrono::steady_clock::now();
    state = State::stopped;
  }
};