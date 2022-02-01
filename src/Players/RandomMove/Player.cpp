#include "Player.hpp"
#include "../../Utilities/Utilities.hpp"

namespace random_move {
Player::Player(const Game &, const State &state, const nlohmann::json &data) : _StatePtr(&state) {
    Util::GetJsonValidator("players/random_move.schema.json").validate(data);
    // TODO
}
} // namespace random_move
