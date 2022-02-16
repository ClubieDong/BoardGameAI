#pragma once

#include "../../Utilities/Utilities.hpp"
#include "../Game.hpp"
#include <array>
#include <nlohmann/json-schema.hpp>
#include <nlohmann/json.hpp>
#include <utility>

namespace tic_tac_toe {
struct State : public ::State {
    unsigned char MoveCount;
    std::array<std::array<unsigned char, 3>, 3> Board;

    explicit State() : MoveCount(0), Board() {}
    explicit State(const nlohmann::json &data) {
        std::tie(Board, MoveCount) = Util::Json2Board<3, 3, 2>(data["board"]);
    }
    friend bool operator==(const State &left, const State &right) {
        return left.MoveCount == right.MoveCount && left.Board == right.Board;
    }
};

struct Action : public ::Action {
public:
    unsigned char Row, Col;

    // This constructor does not perform validity checks on the parameters,
    // because such invalid actions are sometimes required, such as in the default action generator.
    explicit Action(unsigned char row, unsigned char col) : Row(row), Col(col) {}
    explicit Action(const nlohmann::json &data) : Row(data["row"]), Col(data["col"]) {}
    friend bool operator==(const Action &left, const Action &right) {
        return left.Row == right.Row && left.Col == right.Col;
    }
};

class Game : public ::Game::CRTP<State, Action> {
public:
    explicit Game(const nlohmann::json &) {}
    virtual std::string_view GetType() const override { return "tic_tac_toe"; }

    virtual nlohmann::json GetJsonOfState(const ::State &state_) const override {
        const auto &state = static_cast<const State &>(state_);
        return {{"board", state.Board}};
    }
    virtual nlohmann::json GetJsonOfAction(const ::Action &action_) const override {
        const auto &action = static_cast<const Action &>(action_);
        return {{"row", action.Row}, {"col", action.Col}};
    }

    virtual bool IsValidAction(const ::State &state_, const ::Action &action_) const override {
        const auto &state = static_cast<const State &>(state_);
        const auto &action = static_cast<const Action &>(action_);
        return action.Row < 3 && action.Col < 3 && state.Board[action.Row][action.Col] == 0;
    }
    virtual unsigned int GetNextPlayer(const ::State &state_) const override {
        const auto &state = static_cast<const State &>(state_);
        return state.MoveCount & 1;
    }
    virtual std::optional<std::vector<double>> TakeAction(::State &state, const ::Action &action) const override;
};
} // namespace tic_tac_toe
