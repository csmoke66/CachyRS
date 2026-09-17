#pragma once
#include <exception>
#include <optional>
#include <string>
#include <thread>

namespace crs
{
  class OwnershipException : public std::runtime_error
  {
  public:
    using std::runtime_error::runtime_error;
  };

  template <typename T>
  class ThreadOwned
  {
  private:
    std::optional<std::thread::id> owner;
    T raw{};

  public:
    ThreadOwned(std::thread::id owner) : owner(owner)
    {
    }

    ThreadOwned(std::thread::id owner, const T &raw) : owner(owner),
                                                       raw(raw)
    {
    }

    ThreadOwned(const T &raw) : raw(raw)
    {
    }

    ThreadOwned(const ThreadOwned<T> &o) = default;

  private:
    void check_ownership() const
    {
      if (owner.has_value() && owner != std::this_thread::get_id())
      {
        throw OwnershipException("Invalid thread for access");
      }
    }

  public:
    T &operator*()
    {
      check_ownership();
      return raw;
    }

    const T &operator*() const
    {
      check_ownership();
      return raw;
    }

    T operator->()
    {
      check_ownership();
      return raw;
    }

    const T operator->() const
    {
      check_ownership();
      return raw;
    }

    operator void *() const
    {
      check_ownership();
      return raw;
    }

  public:
    T unwrap()
    {
      check_ownership();
      return raw;
    }

    T unwrap_unsafe()
    {
      return raw;
    }
  };
} // namespace crs