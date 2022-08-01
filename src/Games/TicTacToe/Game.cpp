#include "Game.hpp"
#include <cassert>

namespace tic_tac_toe {
std::optional<std::vector<float>> Game::TakeAction(::State &state_, const ::Action &action_) const {
    auto &state = static_cast<State &>(state_);
    const auto &action = static_cast<const Action &>(action_);
    assert(IsValidAction(state, action));

    const auto nextPlayer = GetNextPlayer(state);
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

    std::optional<std::vector<float>> res;
    if (win) {
        res.emplace(2, 0.0f);
        (*res)[nextPlayer] = 1.0f;
    } else if (state.MoveCount == 9) // Draw
        res.emplace(2, 0.5f);
    return res;
}
} // namespace tic_tac_toe
