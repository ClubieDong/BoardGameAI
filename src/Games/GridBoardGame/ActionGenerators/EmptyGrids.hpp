#pragma once

#include "../../ActionGenerator.hpp"
#include <cassert>
#include <numeric>

namespace grid_board_game::action_generator {
template <typename State, typename Action, typename Data>
class EmptyGrids : public ::ActionGenerator::CRTP<Data> {
private:
    const State *m_State;

public:
    explicit EmptyGrids(const ::Game &, const ::State &state, const nlohmann::json &)
        : m_State(static_cast<const State *>(&state)) {}

    virtual std::unique_ptr<::Action> FirstAction(const ::ActionGenerator::Data &data) const override {
        auto action = std::make_unique<Action>(-1);
        [[maybe_unused]] const auto isValid = NextAction(data, *action);
        assert(isValid);
        return action;
    }
    virtual bool NextAction(const ::ActionGenerator::Data &, ::Action &action_) const override {
        using BoardType = typename decltype(m_State->BitBoard)::value_type;
        auto &action = static_cast<Action &>(action_);
        const auto emptyBits =
            ~std::accumulate(m_State->BitBoard.cbegin(), m_State->BitBoard.cend(), BoardType(),
                             [](const BoardType &left, const BoardType &right) { return left | right; });
        while (true) {
            ++action.Position;
            if (emptyBits[action.Position])
                return true;
            if (action.Position >= emptyBits.size())
                return false;
        }
    }
};
} // namespace grid_board_game::action_generator
