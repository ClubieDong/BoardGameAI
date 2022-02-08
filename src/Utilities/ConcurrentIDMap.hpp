#pragma once

#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

// Hash map with auto-incrementing ID as keys,
// supporting thread-safe insertion, erasure, access, and parallel traversal
template <typename T>
class ConcurrentIDMap {
private:
    struct Record {
        // Used to protect the value from being removed when accessing
        mutable std::shared_mutex Mtx;
        std::optional<T> Value;

        template <typename... TArgs>
        explicit Record(TArgs &&... args) : Value(std::in_place, std::forward<TArgs>(args)...) {}

        ~Record() {
            const std::scoped_lock lock(Mtx);
            Value.reset();
        }
    };

    // Used to lock the map and count
    mutable std::shared_mutex m_Mtx;
    unsigned int m_Count = 0;
    std::unordered_map<unsigned int, std::unique_ptr<Record>> m_Map;

public:
    ~ConcurrentIDMap() {
        const std::scoped_lock lock(m_Mtx);
        // Do not use tbb::parallel_for_each as destructors are most likely not CPU intensive
        std::vector<std::thread> threads;
        for (const auto &item : m_Map)
            threads.emplace_back([&]() { item.second->Value.reset(); });
        for (auto &thread : threads)
            thread.join();
    }

    template <typename... TArgs>
    unsigned int Emplace(TArgs &&... args) {
        // Construct the object first, then move it into the map
        auto record = std::make_unique<Record>(std::forward<TArgs>(args)...);
        {
            const std::scoped_lock lock(m_Mtx);
            const auto id = ++m_Count;
            m_Map.try_emplace(id, std::move(record));
            return id;
        }
    }

    void Erase(unsigned int id) {
        // Extract the object from the map first, then destruct it
        typename decltype(m_Map)::node_type node;
        {
            const std::scoped_lock lock(m_Mtx);
            node = m_Map.extract(id);
        }
        // The destructor of the object is implicitly called here
    }

    template <typename Func>
    void Access(unsigned int id, Func func) const {
        Record *record;
        {
            const std::shared_lock lock(m_Mtx);
            record = m_Map.at(id).get();
            // Acquiring the element-wise reader lock here will never be blocked
            record->Mtx.lock_shared();
        }
        {
            const std::shared_lock lock(record->Mtx, std::adopt_lock);
            func(*record->Value);
        }
    }

    template <typename Func>
    void ForEachParallel(Func func) const {
        const std::shared_lock lock(m_Mtx);
        // Do not use tbb::parallel_for_each as destructors are most likely not CPU intensive
        std::vector<std::thread> threads;
        for (const auto &item : m_Map)
            threads.emplace_back([&]() {
                // No element-wise lock is needed here, because we've already got the reader lock of the entire map
                func(*item.second->Value);
            });
        for (auto &thread : threads)
            thread.join();
    }
};
