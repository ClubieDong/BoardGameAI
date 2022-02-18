#include "Game.hpp"
#include <cassert>

namespace tic_tac_toe {
std::optional<std::vector<double>> Game::TakeAction(::State &state_, const ::Action &action_) const {
    auto &state = static_cast<State &>(state_);
    const auto &action = static_cast<const Action &>(action_);
    assert(IsValidAction(state, action));

    const auto nextPlayer = GetNextPlayer(state_);
    state.SetGrid(action.Position, nextPlayer);
    ++state.MoveCount;
    const auto &bitset = state.BitBoard[nextPlayer];
    const bool win = (bitset[0] & bitset[1] & bitset[2]) | // Row #1
                     (bitset[3] & bitset[4] & bitset[5]) | // Row #2
                     (bitset[6] & bitset[7] & bitset[8]) | // Row #3
                     (bitset[0] & bitset[3] & bitset[6]) | // Column #1
                     (bitset[1] & bitset[4] & bitset[7]) | // Column #2
                     (bitset[2] & bitset[5] & bitset[8]) | // Column #3
                     (bitset[0] & bitset[4] & bitset[8]) | // Main diagonal
                     (bitset[2] & bitset[4] & bitset[6]);  // Counter diagonal
    if (win) {
        std::optional<std::vector<double>> res(std::in_place, 2, 0.0);
        (*res)[nextPlayer] = 1.0;
        return res;
    }
    if (state.MoveCount == 9) // Draw
        return std::optional<std::vector<double>>(std::in_place, 2, 0.5);
    return std::nullopt;
}
} // namespace tic_tac_toe
