#pragma once

#include "../GridBoardGame/Game.hpp"

namespace m_n_k_game {
template <unsigned char RowCount, unsigned char ColCount, unsigned char Renju>
class Game : public grid_board_game::Game<RowCount, ColCount, 2> {
public:
    virtual unsigned char GetNextPlayer(const ::Game::State &state_) const {
        const auto &state = static_cast<const typename Game::State &>(state_);
        return state.MoveCount % 2;
    }

    virtual bool IsValidAction(const ::Game::State &state_, const ::Game::Action &action_) const {
        const auto &state = static_cast<const typename Game::State &>(state_);
        const auto &action = static_cast<const typename Game::Action &>(action_);
        return grid_board_game::Game<RowCount, ColCount, 2>::IsValidAction(state, action) &&
               state.GetGrid(action.Position) == 0;
    }

    virtual std::optional<std::vector<float>> TakeAction(::Game::State &state_, const ::Game::Action &action_) const {
        static constexpr std::array<signed char, 4> DX = {0, 1, 1, 1};
        static constexpr std::array<signed char, 4> DY = {1, 0, 1, -1};
        auto &state = static_cast<typename Game::State &>(state_);
        const auto &action = static_cast<const typename Game::Action &>(action_);
        assert(IsValidAction(state, action));
        // Apply the action on the state
        const auto nextPlayer = GetNextPlayer(state);
        state.SetGrid(action.Position, nextPlayer, false);
        ++state.MoveCount;
        // Check if the game is over
        const auto &bitset = state.BitBoards[nextPlayer];
        const auto row = action.GetRow(), col = action.GetCol();
        bool win = false;
        for (unsigned char dire = 0; dire < 4; ++dire) {
            unsigned char count = 0;
            for (auto x = row + DX[dire], y = col + DY[dire];
                 0 < x && x < RowCount && 0 < y && y < ColCount && bitset[x * ColCount + y];
                 x += DX[dire], y += DY[dire])
                ++count;
            for (auto x = row - DX[dire], y = col - DY[dire];
                 0 < x && x < RowCount && 0 < y && y < ColCount && bitset[x * ColCount + y];
                 x -= DX[dire], y -= DY[dire])
                ++count;
            if (count + 1 >= Renju) {
                win = true;
                break;
            }
        }
        // Build result
        std::optional<std::vector<float>> res;
        if (win) {
            res.emplace(2, 0.0f);
            (*res)[nextPlayer] = 1.0f;
        } else if (state.MoveCount == RowCount * ColCount) // Draw
            res.emplace(2, 0.5f);
        return res;
    }
};
} // namespace m_n_k_game
