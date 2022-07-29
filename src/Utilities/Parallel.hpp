#pragma once

#include <future>
#include <thread>
#include <type_traits>
#include <vector>

class Parallel {
public:
    template <typename Func>
    [[nodiscard]] static std::future<std::invoke_result_t<Func>> Async(Func func) {
        return std::async(std::launch::async, func);
    }

    // This function works the same as the parallel version of std::for_each, but is guaranteed not to use a thread pool
    template <typename TContainer, typename Func>
    static void ForEach(const TContainer &container, Func func) {
        std::vector<std::future<void>> futures;
        for (auto &item : container)
            futures.push_back(Async([&]() { func(item); }));
        for (auto &future : futures)
            future.get();
    }
};
