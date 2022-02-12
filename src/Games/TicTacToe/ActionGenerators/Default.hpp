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

    explicit Default(const ::Game &game, const ::State &state, const nlohmann::json &data);

    virtual std::unique_ptr<::Action> FirstAction(const ::ActionGenerator::Data &data) const override;
    virtual bool NextAction(const ::ActionGenerator::Data &, ::Action &action_) const override {
        auto &action = static_cast<Action &>(action_);
        return Util::NextEmptyGrid(m_State->Board, action.Row, action.Col);
    }
};
} // namespace tic_tac_toe::action_generator
