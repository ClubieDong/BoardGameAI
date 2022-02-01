#pragma once

#include "../../Utilities/Utilities.hpp"
#include "../Game.hpp"
#include <array>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>

namespace tic_tac_toe {
class State : public ::State {
public:
    unsigned char MoveCount;
    std::array<std::array<unsigned char, 3>, 3> Board;

    explicit State(const ::Game &) : MoveCount(0), Board() {}
    explicit State(const nlohmann::json &data);
    virtual nlohmann::json GetJson() const override { return {{"board", Board}}; }
};

class Action : public ::Action {
public:
    unsigned char Row, Col;

    // This constructor does not perform validity checks on the parameters,
    // because such invalid actions are sometimes required, such as in the default action generator.
    explicit Action(unsigned char row, unsigned char col) : Row(row), Col(col) {}
    explicit Action(const nlohmann::json &data);
    virtual nlohmann::json GetJson() const override { return {{"row", Row}, {"col", Col}}; }
};

class Game : public ::Game {
public:
    explicit Game(const nlohmann::json &data);
    virtual bool IsValidAction(const ::State &_state, const ::Action &_action) const override {
        const auto &state = static_cast<const State &>(_state);
        const auto &action = static_cast<const Action &>(_action);
        return action.Row < 3 && action.Col < 3 && state.Board[action.Row][action.Col] == 0;
    }
    virtual std::optional<std::vector<double>> TakeAction(::State &_state, const ::Action &_action) const override;
};
} // namespace tic_tac_toe
