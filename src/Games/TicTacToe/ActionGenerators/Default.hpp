#pragma once

#include "../../../Utilities/Utilities.hpp"
#include "../../ActionGenerator.hpp"
#include "../Game.hpp"

class Game;
struct State;
struct Action;

namespace tic_tac_toe::action_generator {
namespace data {
struct Default : ::ActionGenerator::Data {
    friend bool operator==(const Data &, const Data &) { return true; }
};
} // namespace data

class Default : public ::ActionGenerator::CRTP<data::Default> {
private:
    const State *m_State;

public:
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
