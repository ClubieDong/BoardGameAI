#pragma once

#include "../GridBoardGame/Game.hpp"

namespace gobang {
struct State : public grid_board_game::State<15, 15, 2> {
    using grid_board_game::State<15, 15, 2>::State;
};

struct Action : public grid_board_game::Action<15, 15> {
    using grid_board_game::Action<15, 15>::Action;
};

class Game : public grid_board_game::Game<State, Action> {
public:
    explicit Game(const nlohmann::json &) {}
    virtual std::string_view GetType() const override { return "gobang"; }
    virtual std::optional<std::vector<float>> TakeAction(::State &state, const ::Action &action) const override;
};
} // namespace gobang
