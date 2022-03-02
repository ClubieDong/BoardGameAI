#pragma once

#include <bitset>
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

    template <unsigned char RowCount, unsigned char ColCount, unsigned char Top, unsigned char Bottom,
              unsigned char Left, unsigned char Right>
    static constexpr std::bitset<RowCount * ColCount> GetBitMask() {
        std::bitset<RowCount * ColCount> mask;
        for (unsigned char row = 0; row < Top; ++row)
            for (unsigned char col = 0; col < ColCount; ++col)
                mask[row * ColCount + col] = true;
        for (unsigned char row = RowCount - 1; row >= RowCount - Bottom; --row)
            for (unsigned char col = 0; col < ColCount; ++col)
                mask[row * ColCount + col] = true;
        for (unsigned char col = 0; col < Left; ++col)
            for (unsigned char row = 0; row < RowCount; ++row)
                mask[row * ColCount + col] = true;
        for (unsigned char col = ColCount - 1; col >= ColCount - Right; --col)
            for (unsigned char row = 0; row < RowCount; ++row)
                mask[row * ColCount + col] = true;
        return mask;
    }

public:
    template <std::uint64_t Max>
    using UIntBySize = std::invoke_result_t<decltype(UIntBySizeHelper<Max>)>;

    static const nlohmann::json_schema::json_validator &GetJsonValidator(const std::string &path);

    static std::mt19937 &GetRandomEngine() {
        static thread_local std::mt19937 engine(std::random_device{}());
        return engine;
    }

    template <unsigned char RowCount, unsigned char ColCount, unsigned char Top, unsigned char Bottom,
              unsigned char Left, unsigned char Right>
    inline static const std::bitset<RowCount * ColCount>
        BitMask = GetBitMask<RowCount, ColCount, Top, Bottom, Left, Right>();

    template <unsigned char RowCount, unsigned char ColCount, signed char DX, signed char DY>
    static std::bitset<RowCount * ColCount> BitBoardShift(const std::bitset<RowCount * ColCount> &bitBoard) {
        constexpr int shift = DX * ColCount + DY;
        constexpr unsigned char top = gcem::max(0, -DX), bottom = gcem::max(0, DX);
        constexpr unsigned char left = gcem::max(0, -DY), right = gcem::max(0, DY);
        const auto masked = bitBoard & ~BitMask<RowCount, ColCount, top, bottom, left, right>;
        if constexpr (shift > 0)
            return masked << shift;
        else if constexpr (shift < 0)
            return masked >> -shift;
        else
            return bitBoard;
    }
};
