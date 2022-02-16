#include "Default.hpp"
#include <cassert>

namespace tic_tac_toe::action_generator {
std::unique_ptr<::Action> Default::FirstAction(const ::ActionGenerator::Data &data) const {
    auto action = std::make_unique<Action>(0, -1);
    [[maybe_unused]] const auto isValid = NextAction(data, *action);
    assert(isValid);
    return action;
}
} // namespace tic_tac_toe::action_generator
