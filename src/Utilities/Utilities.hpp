#pragma once

#include <gcem.hpp>
#include <nlohmann/json-schema.hpp>
#include <random>
#include <string>
#include <type_traits>

class Util {
private:
    template <std::uint64_t Max>
    static constexpr auto UIntBySizeHelper() {
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
    template <std::uint64_t Max>
    using UIntBySize = std::invoke_result_t<decltype(UIntBySizeHelper<Max>)>;

    static const nlohmann::json_schema::json_validator &GetJsonValidator(const std::string &path);

    static std::mt19937 &GetRandomEngine() {
        static thread_local std::mt19937 engine(std::random_device{}());
        return engine;
    }
};
