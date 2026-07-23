#pragma once
#include <thread>
#include <exception>

namespace crs
{
    class OwnershipException : public std::runtime_error
    {
    public:
        OwnershipException(const std::string &message) : std::runtime_error(message)
        {
        }
    };

    template <typename T>
    class ThreadOwned
    {
    private:
        std::thread::id owner;
        T raw;

    public:
        ThreadOwned(std::thread::id owner)
        {
            this->owner = owner;
        }

        ThreadOwned(std::thread::id owner, const T &raw)
        {
            this->owner = owner;
            this->raw = raw;
        }

        ThreadOwned(const ThreadOwned<T> &o)
        {
            this->owner = o.owner;
            this->raw = o.raw;
        }

    private:
        void check_ownership()
        {
            if (this->owner != std::this_thread::get_id())
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
    };
}