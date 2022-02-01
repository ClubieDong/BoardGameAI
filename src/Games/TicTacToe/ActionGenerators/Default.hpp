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
    const State *_StatePtr;

public:
    explicit Default(const ::Game &game, const ::State &state, const nlohmann::json &data);

    virtual std::unique_ptr<::Action> FirstAction(const ::ActionGenerator::Data &data) const override;
    virtual bool NextAction(const ::ActionGenerator::Data &, ::Action &_action) const override {
        auto &action = static_cast<Action &>(_action);
        return Util::NextEmptyGrid(_StatePtr->Board, action.Row, action.Col);
    }
};
} // namespace tic_tac_toe::action_generator
