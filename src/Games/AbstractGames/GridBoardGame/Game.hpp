#pragma once

#include "../../../Utilities/Utilities.hpp"
#include "../../Game.hpp"
#include <array>
#include <bitset>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace grid_board_game {
template <unsigned char RowCount, unsigned char ColCount, unsigned char PlayerCount>
class Game : public ::Game {
public:
    using PosType = Util::UIntByValue<RowCount * ColCount>;

    struct State : public ::Game::State {
        // Since alignof(State) is 8 most of the time, using a smaller integer type will not save memory
        uint64_t MoveCount = 0;
        std::array<std::bitset<RowCount * ColCount>, PlayerCount> BitBoards = {};

        State() = default;
        explicit State(const nlohmann::json &data) {
            const auto &board = data["board"];
            // TODO: MoveCount
            if (board.size() != RowCount)
                throw std::invalid_argument("The number of board rows does not match");
            for (unsigned char rowIdx = 0; rowIdx < RowCount; ++rowIdx) {
                const auto &row = board[rowIdx];
                if (row.size() != ColCount)
                    throw std::invalid_argument("The number of board columns does not match");
                for (unsigned char colIdx = 0; colIdx < ColCount; ++colIdx) {
                    unsigned char player = row[colIdx];
                    if (player == 0)
                        continue;
                    if (player > PlayerCount)
                        throw std::invalid_argument("The grid value exceeds the number of players");
                    SetGrid(rowIdx * ColCount + colIdx, player - 1, false);
                    ++MoveCount;
                }
            }
        }

        // Return 0 if it's an empty grid, otherwise playerIdx+1
        unsigned char GetGrid(PosType position) const {
            for (unsigned char playerIdx = 0; playerIdx < PlayerCount; ++playerIdx)
                if (BitBoards[playerIdx][position])
                    return playerIdx + 1;
            return 0;
        }
        void SetGrid(PosType position, unsigned char playerIdx, bool clearOtherBits) {
            if (clearOtherBits)
                for (auto &bitBoard : BitBoards)
                    bitBoard[position] = false;
            BitBoards[playerIdx][position] = true;
        }
        std::array<std::array<unsigned char, ColCount>, RowCount> GetBoard() const {
            std::array<std::array<unsigned char, ColCount>, RowCount> board;
            for (unsigned char rowIdx = 0; rowIdx < RowCount; ++rowIdx)
                for (unsigned char colIdx = 0; colIdx < ColCount; ++colIdx)
                    board[rowIdx][colIdx] = GetGrid(rowIdx * ColCount + colIdx);
            return board;
        }

        friend bool operator==(const State &left, const State &right) {
            return left.MoveCount == right.MoveCount && left.BitBoards == right.BitBoards;
        }

        virtual std::unique_ptr<::Game::State> Clone() const override { return std::make_unique<State>(*this); }
        virtual bool Equal(const ::Game::State &state) const override {
            return *this == static_cast<const State &>(state);
        }
        virtual nlohmann::json GetJson() const override { return {{"board", GetBoard()}}; } // TODO: Add MoveCount
    };

    struct Action : public ::Game::Action {
    public:
        PosType Position;

        explicit Action() = default;
        explicit Action(PosType position) : Position(position) {}
        explicit Action(unsigned char row, unsigned char col) : Action(row * ColCount + col) {}
        explicit Action(const nlohmann::json &data) {
            const unsigned char row = data["row"], col = data["col"];
            if (row > RowCount)
                throw std::invalid_argument("Row index exceeds row count");
            if (col > ColCount)
                throw std::invalid_argument("Column index exceeds column count");
            Position = row * ColCount + col;
        }

        unsigned char GetRow() const { return Position / ColCount; }
        unsigned char GetCol() const { return Position % ColCount; }

        friend bool operator==(const Action &left, const Action &right) { return left.Position == right.Position; }

        virtual std::unique_ptr<::Game::Action> Clone() const override { return std::make_unique<Action>(*this); }
        virtual bool Equal(const ::Game::Action &action) const override {
            return *this == static_cast<const Action &>(action);
        }
        virtual nlohmann::json GetJson() const override { return {{"row", GetRow()}, {"col", GetCol()}}; }
    };

    virtual std::unique_ptr<::Game::State> CreateDefaultState() const override { return std::make_unique<State>(); }
    virtual std::unique_ptr<::Game::State> CreateState(const nlohmann::json &data) const override {
        return std::make_unique<State>(data);
    }
    virtual std::unique_ptr<::Game::Action> CreateAction(const nlohmann::json &data) const override {
        return std::make_unique<Action>(data);
    }

    virtual bool IsValidAction(const ::Game::State &, const ::Game::Action &action_) const override {
        const auto &action = static_cast<const Action &>(action_);
        return action.Position < RowCount * ColCount;
    }
};
} // namespace grid_board_game
