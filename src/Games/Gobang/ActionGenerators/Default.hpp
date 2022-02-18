#pragma once

#include "../../GridBoardGame/ActionGenerators/EmptyGrids.hpp"
#include "../Game.hpp"

namespace gobang::action_generator {
namespace data {
struct Default : ::ActionGenerator::Data {
    friend bool operator==(const Default &, const Default &) { return true; }
};
} // namespace data

class Default : public grid_board_game::action_generator::EmptyGrids<State, Action, data::Default> {
public:
    using grid_board_game::action_generator::EmptyGrids<State, Action, data::Default>::EmptyGrids;
    virtual std::string_view GetType() const override { return "gobang/default"; }
};
} // namespace gobang::action_generator
