#pragma once

#include "../GridBoardGame/Game.hpp"

namespace tic_tac_toe {
struct State : public grid_board_game::State<3, 3, 2> {
    using grid_board_game::State<3, 3, 2>::State;
};

struct Action : public grid_board_game::Action<3, 3> {
    using grid_board_game::Action<3, 3>::Action;
};

class Game : public grid_board_game::Game<State, Action> {
public:
    explicit Game(const nlohmann::json &) {}
    virtual std::string_view GetType() const override { return "tic_tac_toe"; }
    virtual std::optional<std::vector<double>> TakeAction(::State &state, const ::Action &action) const override;
};
} // namespace tic_tac_toe
