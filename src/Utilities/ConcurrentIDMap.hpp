#pragma once

#include "Parallel.hpp"
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
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
    unsigned int m_NextID = 1;
    std::unordered_map<unsigned int, std::unique_ptr<Record>> m_Map;

public:
    ~ConcurrentIDMap() {
        const std::scoped_lock lock(m_Mtx);
        // Concurrently release all items
        Parallel::ForEach(m_Map, [](const auto &item) { item.second->Value.reset(); });
    }

    template <typename... TArgs>
    unsigned int Emplace(TArgs &&... args) {
        // Construct the object first, then move it into the map
        auto record = std::make_unique<Record>(std::forward<TArgs>(args)...);
        {
            const std::scoped_lock lock(m_Mtx);
            const auto id = m_NextID++;
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
        Parallel::ForEach(m_Map, [&](const auto &item) { func(*item.second->Value); });
    }
};
