#pragma once

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <random>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>

class Util {
private:
    inline static std::shared_mutex _MtxValidatorMap;
    inline static std::unordered_map<std::string_view, nlohmann::json_schema::json_validator> _ValidatorMap;

public:
    static const nlohmann::json_schema::json_validator &GetJsonValidator(std::string_view path);

    template <std::size_t RowCount, std::size_t ColCount, unsigned char PlayerCount>
    static std::pair<std::array<std::array<unsigned char, ColCount>, RowCount>, unsigned int>
    Json2Board(const nlohmann::json &data) {
        unsigned int count = 0;
        std::array<std::array<unsigned char, ColCount>, RowCount> board;
        Util::GetJsonValidator("basic/board.schema.json").validate(data);
        if (data.size() != RowCount)
            throw std::invalid_argument("The number of board rows does not match");
        for (unsigned int rowIdx = 0; rowIdx < RowCount; ++rowIdx) {
            const auto &row = data[rowIdx];
            if (row.size() != ColCount)
                throw std::invalid_argument("The number of board columns does not match");
            for (unsigned int colIdx = 0; colIdx < ColCount; ++colIdx) {
                unsigned char grid = row[colIdx];
                if (grid > PlayerCount)
                    throw std::invalid_argument("The grid value exceeds the number of players");
                board[rowIdx][colIdx] = row[colIdx];
                if (board[rowIdx][colIdx] != 0)
                    ++count;
            }
        }
        return {board, count};
    }

    template <std::size_t RowCount, std::size_t ColCount>
    static bool NextEmptyGrid(const std::array<std::array<unsigned char, ColCount>, RowCount> &board,
                              unsigned char &row, unsigned char &col) {
        do {
            if (++col >= ColCount)
                ++row, col = 0;
            if (row >= RowCount)
                return false;
        } while (board[row][col] != 0);
        return true;
    }

    static std::mt19937 &GetRandomEngine() {
        static thread_local std::mt19937 engine(std::random_device{}());
        return engine;
    }
};
