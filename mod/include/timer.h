#pragma once
#include <chrono>
#include <thread>

typedef std::chrono::system_clock TimerClock;
class Timer
{
private:
 TimerClock::time_point last_time = TimerClock::now();

public:
  template <typename _Rep, typename _Period>
  inline bool check(const std::chrono::duration<_Rep, _Period> &tp)
  {
    auto now = TimerClock::now();
    auto future = last_time + tp;
    auto passed = now >= future;
    if (passed)
    {
      last_time = now;
    }

    return passed;
  }
};

typedef std::chrono::steady_clock StopwatchClock;
class Stopwatch
{
private:
  enum class State
  {
    stopped,
    running
  };

private:
  StopwatchClock::time_point start = StopwatchClock::now();
  StopwatchClock::time_point end = StopwatchClock::now();
  State state = State::running;

public:
  inline auto check() const
  {
    if (state == State::running)
    {
      return StopwatchClock::now() - start;
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
    start = StopwatchClock::now();
    state = State::running;
  }

  void stop()
  {
    end = StopwatchClock::now();
    state = State::stopped;
  }
};