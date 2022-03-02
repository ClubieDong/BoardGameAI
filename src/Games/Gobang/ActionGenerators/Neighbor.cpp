#include "Neighbor.hpp"
#include "../../../Utilities/Utilities.hpp"

namespace gobang::action_generator {
std::unique_ptr<::ActionGenerator::Data> Neighbor::CreateData(const ::State &state_) const {
    const auto &state = static_cast<const State &>(state_);
    const auto grid = state.BitBoard[0] | state.BitBoard[1];
    auto data = std::make_unique<data::Neighbor>(state);
    for (Action action(0); action.Position < 15 * 15; ++action.Position)
        if (grid[action.Position])
            Update(*data, action);
    return data;
}

std::unique_ptr<::Action> Neighbor::FirstAction(const ::ActionGenerator::Data &data, const ::State &state_) const {
    const auto &state = static_cast<const State &>(state_);
    if (state.MoveCount == 0)
        return std::make_unique<Action>(7, 7);
    auto action = std::make_unique<Action>(-1);
    [[maybe_unused]] const auto isValid = NextAction(data, state, *action);
    assert(isValid);
    return action;
}

bool Neighbor::NextAction(const ::ActionGenerator::Data &data_, const ::State &state_, ::Action &action_) const {
    const auto &data = static_cast<const data::Neighbor &>(data_);
    const auto &state = static_cast<const State &>(state_);
    auto &action = static_cast<Action &>(action_);
    if (state.MoveCount == 0)
        return false;
    for (++action.Position; action.Position < data.InRange.size(); ++action.Position)
        if (data.InRange[action.Position] && !state.BitBoard[0][action.Position] && !state.BitBoard[1][action.Position])
            return true;
    return false;
}

void Neighbor::Update(::ActionGenerator::Data &data_, const ::Action &action_) const {
    auto &data = static_cast<data::Neighbor &>(data_);
    const auto &action = static_cast<const Action &>(action_);
    const auto row = action.GetRow(), col = action.GetCol();
    const unsigned char rowBegin = std::max(0, row - m_Range), rowEnd = std::min(14, row + m_Range);
    const unsigned char colBegin = std::max(0, col - m_Range), colEnd = std::min(14, col + m_Range);
    for (auto row = rowBegin; row <= rowEnd; ++row)
        for (auto col = colBegin; col <= colEnd; ++col)
            data.InRange[row * 15 + col] = true;
}
} // namespace gobang::action_generator
