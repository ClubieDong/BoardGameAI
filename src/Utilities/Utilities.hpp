#pragma once

#include <gcem.hpp>
#include <nlohmann/json-schema.hpp>
#include <random>
#include <string>
#include <type_traits>

class Util {
private:
    template <uint64_t Max>
    static constexpr auto UIntByValueHelper() {
        constexpr unsigned char bit = gcem::ceil(gcem::log2(Max));
        // The `else`s before `if`s are needed to prevent compile error "inconsistent deduction for auto return type"
        if constexpr (bit <= 8)
            return uint8_t{};
        else if constexpr (bit <= 16)
            return uint16_t{};
        else if constexpr (bit <= 32)
            return uint32_t{};
        else if constexpr (bit <= 64)
            return uint64_t{};
    }

public:
    class NonCopyableNonMoveable {
    public:
        NonCopyableNonMoveable() = default;
        NonCopyableNonMoveable(const NonCopyableNonMoveable &) = delete;
        NonCopyableNonMoveable(NonCopyableNonMoveable &&) = delete;
        NonCopyableNonMoveable &operator=(const NonCopyableNonMoveable &) = delete;
        NonCopyableNonMoveable &operator=(NonCopyableNonMoveable &&) = delete;
    };

    // Get the smallest uint type that can hold the value
    template <uint64_t Value>
    using UIntByValue = std::invoke_result_t<decltype(UIntByValueHelper<Value>)>;

    static const nlohmann::json_schema::json_validator &GetJsonValidator(const std::string &path);

    static std::minstd_rand &GetRandomEngine() {
        // The performance of the random engine has a great influence on the efficiency of the MCTS algorithm, I have
        // tested all the random engines provided by the standard library, and `minstd_rand` is the fastest
        static thread_local std::minstd_rand engine(std::random_device{}());
        return engine;
    }
};
