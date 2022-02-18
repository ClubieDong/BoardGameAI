#pragma once

#include "../../Utilities/Utilities.hpp"
#include "../Game.hpp"
#include <array>
#include <bitset>
#include <nlohmann/json.hpp>
#include <stdexcept>

namespace grid_board_game {
template <unsigned char RowCount, unsigned char ColCount, unsigned char PlayerCount>
struct State : public ::State {
    using PosType = Util::UIntBySize<RowCount * ColCount>;

    // TODO: Is it better to use bit operations instead of move count?
    //       e.g. `MoveCount == RowCount * ColCount` vs `(BitBoard[0] | BitBoard[1]).all()`
    // Since alignof(State) is 8 most of the time, using a smaller integer type will not save memory
    std::uint64_t MoveCount = 0;
    std::array<std::bitset<RowCount * ColCount>, PlayerCount> BitBoard = {};

    State() = default;
    explicit State(const nlohmann::json &data) {
        const auto &board = data["board"];
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
                SetGrid(rowIdx * ColCount + colIdx, player - 1);
                ++MoveCount;
            }
        }
    }
    friend bool operator==(const State &left, const State &right) {
        return left.MoveCount == right.MoveCount && left.BitBoard == right.BitBoard;
    }

    // Return 0 if it's an empty grid, otherwise playerIdx+1
    unsigned char GetGrid(PosType position) const {
        for (unsigned char playerIdx = 0; playerIdx < PlayerCount; ++playerIdx)
            if (BitBoard[playerIdx][position])
                return playerIdx + 1;
        return 0;
    }
    void SetGrid(PosType position, unsigned char playerIdx) { BitBoard[playerIdx][position] = true; }
    std::array<std::array<unsigned char, ColCount>, RowCount> GetBoard() const {
        std::array<std::array<unsigned char, ColCount>, RowCount> board;
        for (unsigned char rowIdx = 0; rowIdx < RowCount; ++rowIdx)
            for (unsigned char colIdx = 0; colIdx < ColCount; ++colIdx)
                board[rowIdx][colIdx] = GetGrid(rowIdx * ColCount + colIdx);
        return board;
    }
};

template <unsigned char RowCount, unsigned char ColCount>
struct Action : public ::Action {
public:
    using PosType = Util::UIntBySize<RowCount * ColCount>;
    PosType Position;

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
    friend bool operator==(const Action &left, const Action &right) { return left.Position == right.Position; }

    unsigned char GetRow() const { return Position / ColCount; }
    unsigned char GetCol() const { return Position % ColCount; }
};

template <typename State, typename Action>
class Game : public ::Game::CRTP<State, Action> {
public:
    virtual nlohmann::json GetJsonOfState(const ::State &state_) const override {
        const auto &state = static_cast<const State &>(state_);
        return {{"board", state.GetBoard()}};
    }
    virtual nlohmann::json GetJsonOfAction(const ::Action &action_) const override {
        const auto &action = static_cast<const Action &>(action_);
        return {{"row", action.GetRow()}, {"col", action.GetCol()}};
    }
    virtual bool IsValidAction(const ::State &state_, const ::Action &action_) const override {
        const auto &state = static_cast<const State &>(state_);
        const auto &action = static_cast<const Action &>(action_);
        return action.Position < state.BitBoard[0].size() && state.GetGrid(action.Position) == 0;
    }
    virtual unsigned char GetNextPlayer(const ::State &state_) const override {
        const auto &state = static_cast<const State &>(state_);
        return state.MoveCount % state.BitBoard.size();
    }
};
} // namespace grid_board_game
