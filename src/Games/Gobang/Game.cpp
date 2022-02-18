#include "Game.hpp"
#include <array>
#include <cassert>

static constexpr std::array<signed char, 4> DX = {0, 1, 1, 1};
static constexpr std::array<signed char, 4> DY = {1, 0, 1, -1};

namespace gobang {
std::optional<std::vector<double>> Game::TakeAction(::State &state_, const ::Action &action_) const {
    // TODO: Maybe optimize using bit operations?
    auto &state = static_cast<State &>(state_);
    const auto &action = static_cast<const Action &>(action_);
    assert(IsValidAction(state, action));

    const auto nextPlayer = GetNextPlayer(state);
    state.SetGrid(action.Position, nextPlayer);
    ++state.MoveCount;

    const auto &bitset = state.BitBoard[nextPlayer];
    const auto row = action.GetRow(), col = action.GetCol();
    bool win = false;
    for (unsigned char dire = 0; dire < 4; ++dire) {
        unsigned char count = 0;
        for (auto x = row + DX[dire], y = col + DY[dire]; x < 15 && y < 15 && bitset[x * 15 + y];
             x += DX[dire], y += DY[dire])
            ++count;
        for (auto x = row - DX[dire], y = col - DY[dire]; x < 15 && y < 15 && bitset[x * 15 + y];
             x -= DX[dire], y -= DY[dire])
            ++count;
        if (count >= 4) {
            win = true;
            break;
        }
    }

    std::optional<std::vector<double>> res;
    if (win) {
        res.emplace(2, 0.0);
        (*res)[nextPlayer] = 1.0;
    } else if (state.MoveCount == 15 * 15) // Draw
        res.emplace(2, 0.5);
    return res;
}
} // namespace gobang
