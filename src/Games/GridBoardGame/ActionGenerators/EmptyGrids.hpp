#pragma once

#include "../../ActionGenerator.hpp"
#include <cassert>
#include <numeric>

namespace grid_board_game::action_generator {
template <typename State, typename Action, typename Data>
class EmptyGrids : public ::ActionGenerator::CRTP<Data> {
public:
    explicit EmptyGrids(const ::Game &, const nlohmann::json &) {}

    virtual std::unique_ptr<::Action> FirstAction(const ::ActionGenerator::Data &data,
                                                  const ::State &state) const override {
        auto action = std::make_unique<Action>(-1);
        [[maybe_unused]] const auto isValid = NextAction(data, state, *action);
        assert(isValid);
        return action;
    }
    virtual bool NextAction(const ::ActionGenerator::Data &, const ::State &state_, ::Action &action_) const override {
        const auto &state = static_cast<const State &>(state_);
        auto &action = static_cast<Action &>(action_);
        using BoardType = typename decltype(state.BitBoard)::value_type;
        const auto emptyBits =
            ~std::accumulate(state.BitBoard.cbegin(), state.BitBoard.cend(), BoardType(),
                             [](const BoardType &left, const BoardType &right) { return left | right; });
        for (++action.Position; action.Position < emptyBits.size(); ++action.Position)
            if (emptyBits[action.Position])
                return true;
        return false;
    }
};
} // namespace grid_board_game::action_generator
