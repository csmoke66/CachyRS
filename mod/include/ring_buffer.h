#include <atomic>
#include <cstdint>
#include <algorithm>

#include "util.h"

namespace crs
{
    //
    // A simple ring buffer implementation that assumes 1 consumer and multiple producers.
    //
    // Uses a bitmap, no locking, and no dynamic allocations.
    //
    // The capacity is 64, and push will return false as long as the queue is filled.
    //
    template <typename T>
    class RingBuffer
    {
        struct RingBufferElement
        {
            T t;
            uint64_t index;
        };

    public:
        static constexpr auto size()
        {
            return 64;
        }

    private:
        RingBufferElement buffer[size()];
        std::atomic<uint64_t> bitmap = 0;
        std::atomic<uint64_t> allocator = 0;
        std::atomic<uint64_t> unprocessed = 0;

    public:
        template <typename FN>
        FINLINE void process(FN fn)
        {
            auto bitmap_t = bitmap.exchange(0, std::memory_order_acq_rel);

            std::array<RingBufferElement, size()> sorted;
            size_t sorted_size = 0;
            for (auto i = 0; i < size(); i++)
            {
                auto bit = 1ull << i;
                if (!!(bitmap_t & bit))
                {
                    sorted[sorted_size++] = buffer[i];
                }
            };

            unprocessed.exchange(0, std::memory_order_release);

            // clang-format off
            std::sort(sorted.begin(), sorted.begin() + sorted_size, [](const RingBufferElement& a, const RingBufferElement& b)
            {
                return a.index < b.index;
            });

            for (auto i = sorted.begin(); i < sorted.begin() + sorted_size; i++)
            {
                fn(i->t);
            }
            // clang-format on
        }

        FINLINE bool push(T t)
        {
            auto unprocessed_t = unprocessed.fetch_add(1, std::memory_order_relaxed);
            if (unprocessed_t >= size())
            {
                return false;
            }

            auto idx = allocator.fetch_add(1, std::memory_order_relaxed);
            buffer[idx % size()] = {t, idx};
            bitmap.fetch_or(1ull << (idx % size()), std::memory_order_release);
            return true;
        }
    };
}