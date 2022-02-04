#include "Game.hpp"
#include "../../Utilities/Utilities.hpp"
#include <cassert>

namespace tic_tac_toe {
State::State(const nlohmann::json &data) {
    Util::GetJsonValidator("states/tic_tac_toe.schema.json").validate(data);
    std::tie(Board, MoveCount) = Util::Json2Board<3, 3, 2>(data["board"]);
}

Action::Action(const nlohmann::json &data) {
    Util::GetJsonValidator("actions/tic_tac_toe.schema.json").validate(data);
    Row = data["row"];
    Col = data["col"];
}

Game::Game(const nlohmann::json &data) {
    Util::GetJsonValidator("games/tic_tac_toe.schema.json").validate(data);
}

std::optional<std::vector<double>> Game::TakeAction(::State &state_, const ::Action &action_) const {
    auto &state = static_cast<State &>(state_);
    const auto &action = static_cast<const Action &>(action_);
    assert(IsValidAction(state, action));

    const auto nextPlayer = (state.MoveCount & 1) + 1;
    state.Board[action.Row][action.Col] = nextPlayer;
    ++state.MoveCount;
    const bool win = (state.Board[0][0] & state.Board[0][1] & state.Board[0][2]) | // Row
                     (state.Board[1][0] & state.Board[1][1] & state.Board[1][2]) |
                     (state.Board[2][0] & state.Board[2][1] & state.Board[2][2]) |
                     (state.Board[0][0] & state.Board[1][0] & state.Board[2][0]) | // Column
                     (state.Board[0][1] & state.Board[1][1] & state.Board[2][1]) |
                     (state.Board[0][2] & state.Board[1][2] & state.Board[2][2]) |
                     (state.Board[0][0] & state.Board[1][1] & state.Board[2][2]) | // Main diagonal
                     (state.Board[0][2] & state.Board[1][1] & state.Board[2][0]);  // Counter diagonal
    if (win) {
        std::optional<std::vector<double>> res(std::in_place, 2, 0.0);
        (*res)[nextPlayer - 1] = 1.0;
        return res;
    }
    if (state.MoveCount == 9) // Draw
        return std::optional<std::vector<double>>(std::in_place, 2, 0.5);
    return std::nullopt;
}
} // namespace tic_tac_toe
