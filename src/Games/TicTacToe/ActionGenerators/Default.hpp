#pragma once

#include "../../ActionGenerator.hpp"
#include "../Game.hpp"
#include <nlohmann/json.hpp>

class Game;
class State;
class Action;

namespace tic_tac_toe::action_generator {
class Default : public ::ActionGenerator {
private:
    const State *m_State;

public:
    class Data : public ::ActionGenerator::Data::CRTP<Data> {
    public:
        friend bool operator==(const Data &, const Data &) { return true; }
    };

    explicit Default(const ::Game &, const ::State &state, const nlohmann::json &)
        : m_State(static_cast<const State *>(&state)) {}

    virtual std::string_view GetType() const override { return "tic_tac_toe/default"; }
    virtual std::unique_ptr<::Action> FirstAction(const ::ActionGenerator::Data &data) const override;
    virtual bool NextAction(const ::ActionGenerator::Data &, ::Action &action_) const override {
        auto &action = static_cast<Action &>(action_);
        return Util::NextEmptyGrid(m_State->Board, action.Row, action.Col);
    }
};
} // namespace tic_tac_toe::action_generator
